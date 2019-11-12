#include "stdafx.h"
#include "RenderGraph.h"

#include "Renderer/Utils/Stringify.h"

#define LOG_TAG MANTIS_TEXT("RenderGraph")

namespace Mantis
{
    static const RenderGraphQueue COMPUTE_QUEUES = RenderGraphQueue::AsyncCompute | RenderGraphQueue::Compute;

    RenderGraph::RenderGraph()
    {
        EVENT_MANAGER_REGISTER_LATCH(RenderGraph, on_swapchain_changed, on_swapchain_destroyed, Vulkan::SwapchainParameterEvent);
    }

    void RenderGraph::Reset()
    {
        m_passes.clear();
        m_resources.clear();
        m_passToIndex.clear();
        m_resourceToIndex.clear();
        m_physicalPasses.clear();
        m_physicalDimensions.clear();
        m_physicalAttachments.clear();
        m_physicalBuffers.clear();
        m_physicalImageAttachments.clear();
        m_physicalEvents.clear();
        m_physicalHistoryEvents.clear();
        m_physicalHistoryImageAttachments.clear();
    }

    void RenderGraph::OnSwapchainChanged(const Vulkan::SwapchainParameterEvent&)
    {
    }

    void RenderGraph::OnSwapchainDestroyed(const Vulkan::SwapchainParameterEvent&)
    {
        m_physicalImageAttachments.clear();
        m_physicalHistoryImageAttachments.clear();
        m_physicalEvents.clear();
        m_physicalHistoryEvents.clear();
    }

    RenderTextureResource& RenderGraph::GetTextureResource(const String& name)
    {
        auto itr = m_resourceToIndex.find(name);
        if (itr != end(m_resourceToIndex))
        {
            assert(m_resources[itr->second]->GetType() == RenderResource::Type::Texture);
            return static_cast<RenderTextureResource&>(*m_resources[itr->second]);
        }
        else
        {
            uint32_t index = m_resources.size();
            m_resources.emplace_back(new RenderTextureResource(name, index));
            m_resourceToIndex[name] = index;
            return static_cast<RenderTextureResource&>(*m_resources.back());
        }
    }

    RenderBufferResource& RenderGraph::GetBufferResource(const String& name)
    {
        auto itr = m_resourceToIndex.find(name);
        if (itr != end(m_resourceToIndex))
        {
            assert(m_resources[itr->second]->GetType() == RenderResource::Type::Buffer);
            return static_cast<RenderBufferResource&>(*m_resources[itr->second]);
        }
        else
        {
            uint32_t index = m_resources.size();
            m_resources.emplace_back(new RenderBufferResource(name, index));
            m_resourceToIndex[name] = index;
            return static_cast<RenderBufferResource&>(*m_resources.back());
        }
    }

    eastl::vector<eastl::shared_ptr<Buffer>> RenderGraph::ConsumePhysicalBuffers() const
    {
        return m_physicalBuffers;
    }

    void RenderGraph::InstallPhysicalBuffers(eastl::vector<eastl::shared_ptr<Buffer>> buffers)
    {
        m_physicalBuffers = move(buffers);
    }

    eastl::shared_ptr<Buffer> RenderGraph::ConsumePersistentPhysicalBufferResource(uint32_t index) const
    {
        if (index >= m_physicalBuffers.size())
        {
            return nullptr;
        }
        if (!m_physicalBuffers[index])
        {
            return nullptr;
        }
        return m_physicalBuffers[index];
    }

    void RenderGraph::InstallPersistentPhysicalBufferResource(uint32_t index, eastl::shared_ptr<Buffer> buffer)
    {
        if (index >= m_physicalBuffers.size())
        {
            Logger::ErrorTF(LOG_TAG, "Cannot install phyiscal buffers, index out of range: %u", index);
            return;
        }
        m_physicalBuffers[index] = buffer;
    }

    RenderPass& RenderGraph::AddPass(const String& name, RenderGraphQueue queue)
    {
        auto itr = m_passToIndex.find(name);
        if (itr != end(m_passToIndex))
        {
            return *m_passes[itr->second];
        }
        else
        {
            uint32_t index = m_passes.size();
            m_passes.emplace_back(new RenderPass(*this, name, index, queue));
            m_passToIndex[name] = index;
            return *m_passes.back();
        }
    }

    void RenderGraph::SetBackbufferSource(const String& name)
    {
        m_backbufferSource = name;
    }

    void RenderGraph::Bake()
    {
        // reset state
        m_passStack.clear();

        m_passDependencies.clear();
        m_passDependencies.resize(m_passes.size());

        m_passMergeDependencies.clear();
        m_passMergeDependencies.resize(m_passes.size());

        // first validate that the graph is sane
        ValidatePasses();

        // work our way back from the backbuffer, and sort out all the dependencies
        auto itr = m_resourceToIndex.find(m_backbufferSource);
        if (itr == end(m_resourceToIndex))
        {
            Logger::ErrorT(LOG_TAG, "Backbuffer source does not exist!");
        }

        auto& backbufferResource = *m_resources[itr->second];

        if (backbufferResource.GetWritePasses().empty())
        {
            Logger::ErrorTF(LOG_TAG, "No pass exists which writes to resource \"%s\"!", backbufferResource.GetName());
        }

        for (auto& pass : backbufferResource.GetWritePasses())
        {
            m_passStack.push_back(pass);
        }

        auto tmpPassStack = m_passStack;
        for (auto& pushedPass : tmpPassStack)
        {
            TraverseDependencies(*m_passes[pushedPass], 0);
        }

        eastl::reverse(begin(m_passStack), end(m_passStack));
        FilterPasses(m_passStack);

        // reorder passes to extract better pipelining
        ReorderPasses(m_passStack);

        // Figure out which physical resources are needed. Here we will alias resources which can trivially alias via renaming.
        BuildPhysicalResources();

        // try to merge adjacent passes together
        BuildPhysicalPasses();

        // after merging physical passes and resources, if an image resource is only used in a single physical pass, make it transient
        BuildTransients();

        // now that we are done we can make render passes
        BuildRenderPassInfo();

        // for each render pass in isolation, figure out the barriers required
        BuildBarriers();

        // Check if the swapchain needs to be blitted to in case the geometry does not match the backbuffer,
        // or the usage of the image makes that impossible.
        m_swapchainPhysicalIndex = m_resources[m_resourceToIndex[m_backbufferSource]]->GetPhysicalIndex();

        auto& backbufferDim = m_physicalDimensions[m_swapchainPhysicalIndex];

        // If resource is touched in async-compute, we cannot alias with swapchain.
        // If resource is not transient, it's being used in multiple physical passes,
        // we can't use the implicit subpass dependencies for dealing with swapchain.
        bool canAliasBackbuffer = HAS_NO_FLAG(backbufferDim.queues, COMPUTE_QUEUES) && backbufferDim.transient;

        backbufferDim.transient = false;
        backbufferDim.persistent = m_swapchainDimensions.persistent;
        if (!canAliasBackbuffer || backbufferDim != m_swapchainDimensions)
        {
            m_swapchainPhysicalIndex = RenderResource::Unused;
            if ((backbufferDim.queues & RenderGraphQueue::Graphics) == 0)
            {
                backbufferDim.queues |= RenderGraphQueue::AsyncGraphics;
            }
            else
            {
                backbufferDim.queues |= RenderGraphQueue::Graphics;
            }
        }
        else
        {
            m_physicalDimensions[m_swapchainPhysicalIndex].transient = true;
        }

        // Based on our render graph, figure out the barriers we actually need.
        // Some barriers are implicit (transients), and some are redundant, i.e. same texture read in multiple passes.
        BuildPhysicalBarriers();

        // Figure out which images can alias with each other.
        // Also build virtual "transfer" barriers. These things only copy events over to other physical resources.
        BuildAliases();
    }

    void RenderGraph::ValidatePasses()
    {
        for (auto& pass : m_passes)
        {
            if (pass->GetColorInputs().size() != pass->GetColorOutputs().size())
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" failed validation, there are %u color inputs but %u color outputs.",
                    pass->GetName().c_str(),
                    pass->GetColorInputs().size(),
                    pass->GetColorOutputs().size());
            }
            if (pass->GetStorageInputs().size() != pass->GetStorageOutputs().size())
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" failed validation, there are %u storage inputs but %u storage outputs.",
                    pass->GetName().c_str(),
                    pass->GetStorageInputs().size(),
                    pass->GetStorageOutputs().size());
            }
            if (pass->GetBlitTextureInputs().size() != pass->GetBlitTextureOutputs().size())
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" failed validation, there are %u blit texture inputs but %u blit texture outputs.",
                    pass->GetName().c_str(),
                    pass->GetBlitTextureInputs().size(),
                    pass->GetBlitTextureOutputs().size());
            }
            if (pass->GetStorageTextureInputs().size() != pass->GetStorageTextureOutputs().size())
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" failed validation, there are %u storage texture inputs but %u storage texture outputs.",
                    pass->GetName().c_str(),
                    pass->GetStorageTextureInputs().size(),
                    pass->GetStorageTextureInputs().size());
            }
            if (!pass->GetResolveOutputs().empty() && pass->GetResolveOutputs().size() != pass->GetColorOutputs().size())
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" failed validation, there are %u resolve outputs but %u color outputs outputs.",
                    pass->GetName().c_str(),
                    pass->GetResolveOutputs().size(),
                    pass->GetColorOutputs().size());
            }

            for (uint32_t i = 0; i < pass->GetColorInputs().size(); i++)
            {
                if (!pass->GetColorInputs()[i])
                {
                    continue;
                }
                if (GetResourceDimensions(*pass->GetColorInputs()[i]) != GetResourceDimensions(*pass->GetColorOutputs()[i]))
                {
                    pass->MakeColorInputScaled(i);
                }
            }

            for (uint32_t i = 0; i < pass->GetStorageOutputs().size(); i++)
            {
                if (!pass->GetStorageInputs()[i])
                {
                    continue;
                }
                if (pass->GetStorageInputs()[i]->GetBufferInfo() != pass->GetStorageOutputs()[i]->GetBufferInfo())
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" failed validation, input storage buffer \"%s\" does not match dimentions or usage of output storage buffer \"%s\"",
                        pass->GetName().c_str(),
                        pass->GetStorageInputs()[i]->GetName().c_str(),
                        pass->GetStorageOutputs()[i]->GetName().c_str());
                }
            }

            for (uint32_t i = 0; i < pass->GetBlitTextureOutputs().size(); i++)
            {
                if (!pass->GetBlitTextureInputs()[i])
                {
                    continue;
                }
                if (GetResourceDimensions(*pass->GetBlitTextureInputs()[i]) != GetResourceDimensions(*pass->GetBlitTextureOutputs()[i]))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" failed validation, input blit image \"%s\" does not match dimentions or usage of output blit image \"%s\"",
                        pass->GetName().c_str(),
                        pass->GetBlitTextureInputs()[i]->GetName().c_str(),
                        pass->GetBlitTextureOutputs()[i]->GetName().c_str());
                }
            }

            for (uint32_t i = 0; i < pass->GetStorageTextureOutputs().size(); i++)
            {
                if (!pass->GetStorageTextureInputs()[i])
                {
                    continue;
                }
                if (GetResourceDimensions(*pass->GetStorageTextureInputs()[i]) != GetResourceDimensions(*pass->GetStorageTextureOutputs()[i]))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" failed validation, input storage texture \"%s\" does not match dimentions of output storage texture \"%s\"",
                        pass->GetName().c_str(),
                        pass->GetStorageTextureInputs()[i]->GetName().c_str(),
                        pass->GetStorageTextureOutputs()[i]->GetName().c_str());
                }
            }

            if (pass->GetDepthStencilInput() && pass->GetDepthStencilOutput())
            {
                if (GetResourceDimensions(*pass->GetDepthStencilInput()) != GetResourceDimensions(*pass->GetDepthStencilOutput()))
                {
                    Logger::ErrorTF(LOG_TAG, 
                        "Pass \"%s\" failed validation, input depth stencil \"%s\" does not match dimentions of output depth stencil \"%s\"",
                        pass->GetName().c_str(),
                        pass->GetDepthStencilInput()->GetName().c_str(),
                        pass->GetDepthStencilOutput()->GetName().c_str());
                }
            }
        }
    }

    void RenderGraph::TraverseDependencies(const RenderPass& pass, uint32_t stackCount)
    {
        // for these, make sure that we pull in the dependency right away so we can merge render passes if possible
        if (pass.GetDepthStencilInput())
        {
            DependPassesRecursive(pass, pass.GetDepthStencilInput()->GetWritePasses(), stackCount, false, false, true);
        }

        for (auto* input : pass.GetAttachmentInputs())
        {
            bool selfDependency = pass.GetDepthStencilOutput() == input;
            if (eastl::find(begin(pass.GetColorOutputs()), end(pass.GetColorOutputs()), input) != end(pass.GetColorOutputs()))
            {
                selfDependency = true;
            }

            if (!selfDependency)
            {
                DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, true);
            }
        }

        for (auto* input : pass.GetColorInputs())
        {
            if (input)
            {
                DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, true);
            }
        }

        for (auto* input : pass.GetColorScaleInputs())
        {
            if (input)
            {
                DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, false);
            }
        }

        for (auto* input : pass.GetBlitTextureInputs())
        {
            if (input)
            {
                DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, false);
            }
        }

        for (auto& input : pass.GetGenericTextureInputs())
        {
            DependPassesRecursive(pass, input.texture->GetWritePasses(), stackCount, false, false, false);
        }

        for (auto* input : pass.GetStorageInputs())
        {
            if (input)
            {
                // there might be no writers of this resource if it's used in a feedback fashion
                DependPassesRecursive(pass, input->GetWritePasses(), stackCount, true, false, false);
                // deal with write-after-read hazards if a storage buffer is read in other passes
                // (feedback) before being updated
                DependPassesRecursive(pass, input->GetReadPasses(), stackCount, true, true, false);
            }
        }

        for (auto* input : pass.GetStorageTextureInputs())
        {
            if (input)
            {
                DependPassesRecursive(pass, input->GetWritePasses(), stackCount, false, false, false);
            }
        }

        for (auto& input : pass.GetGenericBufferInputs())
        {
            // there might be no writers of this resource if it's used in a feedback fashion
            DependPassesRecursive(pass, input.buffer->GetWritePasses(), stackCount, true, false, false);
        }
    }

    void RenderGraph::DependPassesRecursive(
        const RenderPass& self,
        const eastl::unordered_set<uint32_t>& writtenPasses,
        uint32_t stackCount,
        bool noCheck,
        bool ignoreSelf,
        bool mergeDependency)
    {
        if (!noCheck && writtenPasses.empty())
        {
            Logger::ErrorTF(LOG_TAG, "No pass exists which writes to resources in pass \"%s\"!", self.GetName());
        }

        if (stackCount > m_passes.size())
        {
            Logger::ErrorTF(LOG_TAG, "Dependency cycle detected for pass \"%s\"!", self.GetName());
        }

        for (auto& pass : writtenPasses)
        {
            if (pass != self.GetIndex())
            {
                m_passDependencies[self.GetIndex()].insert(pass);
            }
        }

        if (mergeDependency)
        {
            for (auto& pass : writtenPasses)
            {
                if (pass != self.GetIndex())
                {
                    m_passMergeDependencies[self.GetIndex()].insert(pass);
                }
            }
        }

        stackCount++;

        for (auto& pushedPass : writtenPasses)
        {
            if (ignoreSelf && pushedPass == self.GetIndex())
            {
                continue;
            }
            else if (pushedPass == self.GetIndex())
            {
                Logger::ErrorTF(LOG_TAG, "Pass \"%s\" depends on itself!", self.GetName());
            }

            m_passStack.push_back(pushedPass);
            auto& pass = *m_passes[pushedPass];
            TraverseDependencies(pass, stackCount);
        }
    }

    void RenderGraph::FilterPasses(eastl::vector<uint32_t>& list)
    {
        // remove passes that are not used/redundant
        eastl::unordered_set<uint32_t> seen;

        auto outputItr = begin(list);
        for (auto itr = begin(list); itr != end(list); ++itr)
        {
            if (!seen.count(*itr))
            {
                *outputItr = *itr;
                seen.insert(*itr);
                ++outputItr;
            }
        }

        list.erase(outputItr, end(list));
    }

    void RenderGraph::ReorderPasses(eastl::vector<uint32_t>& flattenedPasses)
    {
        // If a pass depends on an earlier pass via merge dependencies, copy over dependencies
        // to the dependees to avoid cases which can break subpass merging.  This is a "soft"
        // dependency. If we ignore it, it's not a real problem.
        for (auto& passMergeDeps : m_passMergeDependencies)
        {
            auto passIndex = uint32_t(&passMergeDeps - m_passMergeDependencies.data());
            auto& passDeps = m_passDependencies[passIndex];

            for (auto& mergeDep : passMergeDeps)
            {
                for (auto& dependee : passDeps)
                {
                    // avoid cycles
                    if (DependsOnPass(dependee, mergeDep))
                    {
                        continue;
                    }
                    if (mergeDep != dependee)
                    {
                        m_passDependencies[mergeDep].insert(dependee);
                    }
                }
            }
        }

        if (flattenedPasses.size() <= 2)
        {
            return;
        }

        // TODO: This is very inefficient, but should work okay for a reasonable amount of passes
        eastl::vector<uint32_t> unscheduledPasses;
        unscheduledPasses.reserve(m_passes.size());
        eastl::swap(flattenedPasses, unscheduledPasses);

        const auto schedule = [&](uint32_t index)
        {
            // need to preserve the order of remaining elements
            flattenedPasses.push_back(unscheduledPasses[index]);
            eastl::move(unscheduledPasses.begin() + index + 1, unscheduledPasses.end(), unscheduledPasses.begin() + index);
            unscheduledPasses.pop_back();
        };

        schedule(0);
        while (!unscheduledPasses.empty())
        {
            // Find the next pass to schedule.
            // We can pick any pass N, if the pass does not depend on anything left in unscheduledPasses.
            // unscheduledPasses[0] is always okay as a fallback, so unless we find something better,
            // we will at least pick that.

            // Ideally, we pick a pass which does not introduce any hard barrier.
            // A "hard barrier" here is where a pass depends directly on the pass before it forcing something ala vkCmdPipelineBarrier,
            // we would like to avoid this if possible.

            // Find the pass which has the optimal overlap factor which means the number of passes can be scheduled in-between
            // the depender, and the dependee.

            uint32_t bestCandidate = 0;
            uint32_t bestOverlapFactor = 0;

            for (uint32_t i = 0; i < unscheduledPasses.size(); i++)
            {
                uint32_t overlapFactor = 0;

                // Always try to merge passes if possible on tilers.
                // This might not make sense on desktop however,
                // so we can conditionally enable this path depending on our GPU.
                if (m_passMergeDependencies[unscheduledPasses[i]].count(flattenedPasses.back()))
                {
                    overlapFactor = ~0u;
                }
                else
                {
                    for (auto itr = flattenedPasses.rbegin(); itr != flattenedPasses.rend(); ++itr)
                    {
                        if (DependsOnPass(unscheduledPasses[i], *itr))
                        {
                            break;
                        }
                        overlapFactor++;
                    }
                }

                if (overlapFactor <= bestOverlapFactor)
                {
                    continue;
                }

                bool possibleCandidate = true;
                for (uint32_t j = 0; j < i; j++)
                {
                    if (DependsOnPass(unscheduledPasses[i], unscheduledPasses[j]))
                    {
                        possibleCandidate = false;
                        break;
                    }
                }

                if (!possibleCandidate)
                {
                    continue;
                }

                bestCandidate = i;
                bestOverlapFactor = overlapFactor;
            }

            schedule(bestCandidate);
        }
    }

    void RenderGraph::BuildPhysicalResources()
    {
        uint32_t physIndex = 0;

        // find resources which can alias safely
        for (auto& passIndex : m_passStack)
        {
            auto& pass = *m_passes[passIndex];

            for (auto& input : pass.GetGenericBufferInputs())
            {
                if (input.buffer->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*input.buffer));
                    input.buffer->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[input.buffer->GetPhysicalIndex()].queues |= input.buffer->GetUsedQueues();
                    m_physicalDimensions[input.buffer->GetPhysicalIndex()].imageUsage |= input.buffer->GetBufferUsage();
                }
            }

            for (auto& input : pass.GetGenericTextureInputs())
            {
                if (input.texture->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*input.texture));
                    input.texture->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[input.texture->GetPhysicalIndex()].queues |= input.texture->GetUsedQueues();
                    m_physicalDimensions[input.texture->GetPhysicalIndex()].imageUsage |= input.texture->GetImageUsage();
                }
            }

            for (auto* input : pass.GetColorScaleInputs())
            {
                if (input && input->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*input));
                    input->SetPhysicalIndex(physIndex++);
                    m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
                }
                else if (input)
                {
                    m_physicalDimensions[input->GetPhysicalIndex()].queues |= input->GetUsedQueues();
                    m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= input->GetImageUsage();
                    m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
                }
            }

            for (uint32_t i = 0; i < pass.GetColorInputs().size(); i++)
            {
                auto* input = pass.GetColorInputs()[i];
                if (input)
                {
                    if (input->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        m_physicalDimensions.push_back(GetResourceDimensions(*input));
                        input->SetPhysicalIndex(physIndex++);
                    }
                    else
                    {
                        m_physicalDimensions[input->GetPhysicalIndex()].queues |= input->GetUsedQueues();
                        m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= input->GetImageUsage();
                    }

                    auto* output = pass.GetColorOutputs()[i];
                    if (output->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        output->SetPhysicalIndex(input->GetPhysicalIndex());
                    }
                    else if (output->GetPhysicalIndex() != input->GetPhysicalIndex())
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Failed to alias resources \"%s\" and \"%s\" in pass \"%s\", indices already claimed!",
                            input->GetName().c_str(),
                            output->GetName().c_str(),
                            pass.GetName().c_str());
                    }
                }
            }

            for (uint32_t i = 0; i < pass.GetStorageInputs().size(); i++)
            {
                auto* input = pass.GetStorageInputs()[i];
                if (input)
                {
                    if (input->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        m_physicalDimensions.push_back(GetResourceDimensions(*input));
                        input->SetPhysicalIndex(physIndex++);
                    }
                    else
                    {
                        m_physicalDimensions[input->GetPhysicalIndex()].queues |= input->GetUsedQueues();
                        m_physicalDimensions[input->GetPhysicalIndex()].bufferInfo.usage |= input->GetBufferUsage();
                    }

                    auto* output = pass.GetStorageOutputs()[i];
                    if (output->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        output->SetPhysicalIndex(input->GetPhysicalIndex());
                    }
                    else if (output->GetPhysicalIndex() != input->GetPhysicalIndex())
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Failed to alias resources \"%s\" and \"%s\" in pass \"%s\", indices already claimed!",
                            input->GetName().c_str(),
                            output->GetName().c_str(),
                            pass.GetName().c_str());
                    }
                }
            }

            for (uint32_t i = 0; i < pass.GetBlitTextureInputs().size(); i++)
            {
                auto* input = pass.GetBlitTextureInputs()[i];
                if (input)
                {
                    if (input->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        m_physicalDimensions.push_back(GetResourceDimensions(*input));
                        input->SetPhysicalIndex(physIndex++);
                    }
                    else
                    {
                        m_physicalDimensions[input->GetPhysicalIndex()].queues |= input->GetUsedQueues();
                        m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= input->GetImageUsage();
                    }

                    auto* output = pass.GetBlitTextureOutputs()[i];
                    if (output->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        output->SetPhysicalIndex(input->GetPhysicalIndex());
                    }
                    else if (output->GetPhysicalIndex() != input->GetPhysicalIndex())
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Failed to alias resources \"%s\" and \"%s\" in pass \"%s\", indices already claimed!",
                            input->GetName().c_str(),
                            output->GetName().c_str(),
                            pass.GetName().c_str());
                    }
                }
            }

            for (uint32_t i = 0; i < pass.GetStorageTextureInputs().size(); i++)
            {
                auto* input = pass.GetStorageTextureInputs()[i];
                if (input)
                {
                    if (input->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        m_physicalDimensions.push_back(GetResourceDimensions(*input));
                        input->SetPhysicalIndex(physIndex++);
                    }
                    else
                    {
                        m_physicalDimensions[input->GetPhysicalIndex()].queues |= input->GetUsedQueues();
                        m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= input->GetImageUsage();
                    }

                    auto* output = pass.GetStorageTextureOutputs()[i];
                    if (output->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        output->SetPhysicalIndex(input->GetPhysicalIndex());
                    }
                    else if (output->GetPhysicalIndex() != input->GetPhysicalIndex())
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Failed to alias resources \"%s\" and \"%s\" in pass \"%s\", indices already claimed!",
                            input->GetName().c_str(),
                            output->GetName().c_str(),
                            pass.GetName().c_str());
                    }
                }
            }

            for (auto* output : pass.GetColorInputs())
            {
                if (output->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*output));
                    output->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[output->GetPhysicalIndex()].queues |= output->GetUsedQueues();
                    m_physicalDimensions[output->GetPhysicalIndex()].imageUsage |= output->GetImageUsage();
                }
            }

            for (auto* output : pass.GetResolveOutputs())
            {
                if (output->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*output));
                    output->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[output->GetPhysicalIndex()].queues |= output->GetUsedQueues();
                    m_physicalDimensions[output->GetPhysicalIndex()].imageUsage |= output->GetImageUsage();
                }
            }

            for (auto* output : pass.GetStorageOutputs())
            {
                if (output->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*output));
                    output->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[output->GetPhysicalIndex()].queues |= output->GetUsedQueues();
                    m_physicalDimensions[output->GetPhysicalIndex()].bufferInfo.usage |= output->GetBufferUsage();
                }
            }

            for (auto* output : pass.GetBlitTextureOutputs())
            {
                if (output->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*output));
                    output->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[output->GetPhysicalIndex()].queues |= output->GetUsedQueues();
                    m_physicalDimensions[output->GetPhysicalIndex()].imageUsage |= output->GetImageUsage();
                }
            }

            for (auto* output : pass.GetStorageTextureOutputs())
            {
                if (output->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*output));
                    output->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[output->GetPhysicalIndex()].queues |= output->GetUsedQueues();
                    m_physicalDimensions[output->GetPhysicalIndex()].imageUsage |= output->GetImageUsage();
                }
            }

            auto* dsInput = pass.GetDepthStencilInput();
            auto* dsOutput = pass.GetDepthStencilOutput();
            if (dsInput)
            {
                if (dsInput->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*dsInput));
                    dsInput->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[dsInput->GetPhysicalIndex()].queues |= dsInput->GetUsedQueues();
                    m_physicalDimensions[dsInput->GetPhysicalIndex()].imageUsage |= dsInput->GetImageUsage();
                }

                if (dsOutput)
                {
                    if (dsOutput->GetPhysicalIndex() == RenderResource::Unused)
                    {
                        dsOutput->SetPhysicalIndex(dsInput->GetPhysicalIndex());
                    }
                    else if (dsOutput->GetPhysicalIndex() != dsInput->GetPhysicalIndex())
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Failed to alias resources \"%s\" and \"%s\" in pass \"%s\", indices already claimed!",
                            dsInput->GetName().c_str(),
                            dsOutput->GetName().c_str(),
                            pass.GetName().c_str());
                    }

                    m_physicalDimensions[dsOutput->GetPhysicalIndex()].queues |= dsOutput->GetUsedQueues();
                    m_physicalDimensions[dsOutput->GetPhysicalIndex()].imageUsage |= dsOutput->GetImageUsage();
                }
            }
            else if (dsOutput)
            {
                if (dsOutput->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*dsOutput));
                    dsOutput->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[dsOutput->GetPhysicalIndex()].queues |= dsOutput->GetUsedQueues();
                    m_physicalDimensions[dsOutput->GetPhysicalIndex()].imageUsage |= dsOutput->GetImageUsage();
                }
            }

            // Assign input attachments last so they can alias properly with existing color/depth attachments in the
            // same subpass.
            for (auto* input : pass.GetAttachmentInputs())
            {
                if (input->GetPhysicalIndex() == RenderResource::Unused)
                {
                    m_physicalDimensions.push_back(GetResourceDimensions(*input));
                    input->SetPhysicalIndex(physIndex++);
                }
                else
                {
                    m_physicalDimensions[input->GetPhysicalIndex()].queues |= input->GetUsedQueues();
                    m_physicalDimensions[input->GetPhysicalIndex()].imageUsage |= input->GetImageUsage();
                }
            }

            for (auto& pair : pass.GetFakeResourceAliases())
            {
                pair.second->SetPhysicalIndex(pair.first->GetPhysicalIndex());
            }
        }

        // figure out which physical resources need to have history
        m_physicalImageHasHistory.clear();
        m_physicalImageHasHistory.resize(m_physicalDimensions.size());

        for (auto& passIndex : m_passStack)
        {
            auto& pass = *m_passes[passIndex];
            for (auto& input : pass.GetHistoryInputs())
            {
                uint32_t physIndex = input->GetPhysicalIndex();
                if (physIndex == RenderResource::Unused)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "History input \"%s\" in pass \"%s\" is used, but it was never written to!",
                        input->GetName().c_str(),
                        pass.GetName().c_str());
                }
                m_physicalImageHasHistory[physIndex] = true;
            }
        }
    }

    void RenderGraph::BuildPhysicalPasses()
    {
        const auto findAttachment = [](const eastl::vector<RenderTextureResource*>& resourceList, const RenderTextureResource* resource) -> bool 
        {
            if (!resource)
            {
                return false;
            }
            auto itr = eastl::find_if(begin(resourceList), end(resourceList), [resource](const RenderTextureResource* res)
                {
                    return res->GetPhysicalIndex() == resource->GetPhysicalIndex();
                });
            return itr != end(resourceList);
        };

        const auto findBuffer = [](const eastl::vector<RenderBufferResource*>& resourceList, const RenderBufferResource* resource) -> bool
        {
            if (!resource)
            {
                return false;
            }
            auto itr = eastl::find_if(begin(resourceList), end(resourceList), [resource](const RenderBufferResource* res)
                {
                    return res->GetPhysicalIndex() == resource->GetPhysicalIndex();
                });
            return itr != end(resourceList);
        };

        const auto shouldMerge = [&](const RenderPass& prev, const RenderPass& next) -> bool
        {
            if (!RendererConfig::Get().mergeSubpasses)
            {
                return false;
            }

            // can only merge graphics in same queue
            if (HAS_ANY_FLAG(prev.GetQueue(), COMPUTE_QUEUES) || (next.GetQueue() != prev.GetQueue()))
            {
                return false;
            }

            // if a color output needs mip maps generated after the pass we can't merge
            for (auto* output : prev.GetColorOutputs())
            {
                if (m_physicalDimensions[output->GetPhysicalIndex()].levels > 1)
                {
                    return false;
                }
            }

            // need non-local dependency, cannot merge
            for (auto& input : next.GetGenericTextureInputs())
            {
                if (findAttachment(prev.GetColorOutputs(), input.texture))
                {
                    return false;
                }
                if (findAttachment(prev.GetResolveOutputs(), input.texture))
                {
                    return false;
                }
                if (findAttachment(prev.GetStorageTextureOutputs(), input.texture))
                {
                    return false;
                }
                if (findAttachment(prev.GetBlitTextureOutputs(), input.texture))
                {
                    return false;
                }
                if (input.texture && prev.GetDepthStencilOutput() == input.texture)
                {
                    return false;
                }
            }

            // need non-local dependency, cannot merge
            for (auto& input : next.GetGenericBufferInputs())
            {
                if (findBuffer(prev.GetStorageInputs(), input.buffer))
                {
                    return false;
                }
            }

            // need non-local dependency, cannot merge
            for (auto* input : next.GetBlitTextureInputs())
            {
                if (findAttachment(prev.GetBlitTextureInputs(), input))
                {
                    return false;
                }
            }

            // need non-local dependency, cannot merge
            for (auto* input : next.GetStorageInputs())
            {
                if (findBuffer(prev.GetStorageOutputs(), input))
                {
                    return false;
                }
            }

            // need non-local dependency, cannot merge
            for (auto* input : next.GetStorageTextureInputs())
            {
                if (findAttachment(prev.GetStorageTextureOutputs(), input))
                {
                    return false;
                }
            }

            // need non-local dependency, cannot merge
            for (auto* input : next.GetColorScaleInputs())
            {
                if (findAttachment(prev.GetStorageTextureOutputs(), input))
                {
                    return false;
                }
                if (findAttachment(prev.GetBlitTextureOutputs(), input))
                {
                    return false;
                }
                if (findAttachment(prev.GetColorOutputs(), input))
                {
                    return false;
                }
                if (findAttachment(prev.GetResolveOutputs(), input))
                {
                    return false;
                }
            }


            const auto differentAttachment = [](const RenderResource* a, const RenderResource* b)
            {
                return a && b && a->GetPhysicalIndex() != b->GetPhysicalIndex();
            };

            const auto sameAttachment = [](const RenderResource* a, const RenderResource* b)
            {
                return a && b && a->GetPhysicalIndex() == b->GetPhysicalIndex();
            };

            // need a different depth attachment, break up the pass
            if (differentAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilInput()))
            {
                return false;
            }
            if (differentAttachment(next.GetDepthStencilOutput(), prev.GetDepthStencilInput()))
            {
                return false;
            }
            if (differentAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilOutput()))
            {
                return false;
            }
            if (differentAttachment(next.GetDepthStencilOutput(), prev.GetDepthStencilOutput()))
            {
                return false;
            }

            for (auto* input : next.GetColorInputs())
            {
                if (!input)
                {
                    continue;
                }
                if (findAttachment(prev.GetStorageTextureOutputs(), input))
                {
                    return false;
                }
                if (findAttachment(prev.GetBlitTextureOutputs(), input))
                {
                    return false;
                }
            }

            // now that we can merge, check if we should merge

            // keep color on tile
            for (auto* input : next.GetColorInputs())
            {
                if (!input)
                {
                    continue;
                }
                if (findAttachment(prev.GetColorOutputs(), input))
                {
                    return true;
                }
                if (findAttachment(prev.GetResolveOutputs(), input))
                {
                    return true;
                }
            }

            // keep depth on tile
            if (sameAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilInput()) ||
                sameAttachment(next.GetDepthStencilInput(), prev.GetDepthStencilOutput()))
            {
                return true;
            }

            // keep depth attachment or color on-tile
            for (auto* input : next.GetAttachmentInputs())
            {
                if (findAttachment(prev.GetColorOutputs(), input))
                {
                    return true;
                }
                if (findAttachment(prev.GetResolveOutputs(), input))
                {
                    return true;
                }
                if (input && prev.GetDepthStencilOutput() == input)
                {
                    return true;
                }
            }

            // no reason to merge, so don't
            return false;
        };

        m_physicalPasses.clear();

        for (uint32_t index = 0; index < m_passStack.size();)
        {
            uint32_t mergeEnd = index + 1;
            for (; mergeEnd < m_passStack.size(); mergeEnd++)
            {
                bool merge = true;
                for (uint32_t mergeStart = index; mergeStart < mergeEnd; mergeStart++)
                {
                    if (!shouldMerge(*m_passes[m_passStack[mergeStart]], *m_passes[m_passStack[mergeEnd]]))
                    {
                        merge = false;
                        break;
                    }
                }

                if (!merge)
                {
                    break;
                }
            }

            PhysicalPass physicalPass;
            physicalPass.passes.insert(end(physicalPass.passes), begin(m_passStack) + index, begin(m_passStack) + mergeEnd);
            m_physicalPasses.push_back(eastl::move(physicalPass));
            index = mergeEnd;
        }

        // set the pass indices
        for (auto& physPass : m_physicalPasses)
        {
            uint32_t index = uint32_t(&physPass - m_physicalPasses.data());
            for (auto& pass : physPass.passes)
            {
                m_passes[pass]->SetPhysicalPassIndex(index);
            }
        }
    }

    void RenderGraph::BuildTransients()
    {
        eastl::vector<uint32_t> physicalPassUsed(m_physicalDimensions.size());
        for (auto& u : physicalPassUsed)
        {
            u = RenderPass::Unused;
        }

        for (auto& dim : m_physicalDimensions)
        {
            // buffers are never transient
            // storage images are never transient
            dim.transient = !dim.IsBufferLike();

            // images that needs to presist between frames can not be transient
            uint32_t index = uint32_t(&dim - m_physicalDimensions.data());
            if (m_physicalImageHasHistory[index])
            {
                dim.transient = false;
            }

            // check if transients are configured to be used
            if (Image::HasDepth(dim.format) || Image::HasStencil(dim.format))
            {
                if (!RendererConfig::Get().useTransientDepthStencil)
                {
                    dim.transient = false;
                }
            }
            else if (!RendererConfig::Get().useTransientColor)
            {
                dim.transient = false;
            }
        }

        for (auto& resource : m_resources)
        {
            if (resource->GetType() != RenderResource::Type::Texture)
            {
                continue;
            }

            uint32_t physIndex = resource->GetPhysicalIndex();
            if (physIndex == RenderResource::Unused)
            {
                continue;
            }

            for (auto& pass : resource->GetWritePasses())
            {
                uint32_t phys = m_passes[pass]->GetPhysicalPassIndex();
                if (phys != RenderPass::Unused)
                {
                    if (physicalPassUsed[physIndex] != RenderPass::Unused && phys != physicalPassUsed[physIndex])
                    {
                        m_physicalDimensions[physIndex].transient = false;
                        break;
                    }
                    physicalPassUsed[physIndex] = phys;
                }
            }

            for (auto& pass : resource->GetReadPasses())
            {
                uint32_t phys = m_passes[pass]->GetPhysicalPassIndex();
                if (phys != RenderPass::Unused)
                {
                    if (physicalPassUsed[physIndex] != RenderPass::Unused && phys != physicalPassUsed[physIndex])
                    {
                        m_physicalDimensions[physIndex].transient = false;
                        break;
                    }
                    physicalPassUsed[physIndex] = phys;
                }
            }
        }
    }

    void RenderGraph::BuildRenderPassInfo()
    {
        for (auto& physicalPass : m_physicalPasses)
        {
            physicalPass.subpasses.resize(physicalPass.passes.size());
            physicalPass.colorClearRequests.clear();
            physicalPass.depthClearRequest = {};
            
            auto& rp = physicalPass.renderPassInfo;
            rp.subpasses = physicalPass.subpasses.data();
            rp.numSubpasses = physicalPass.subpasses.size();
            rp.clearAttachments = 0;
            rp.loadAttachments = 0;
            rp.storeAttachments = ~0u;

            auto& colors = physicalPass.physicalColorAttachments;
            colors.clear();

            const auto addUniqueColor = [&](uint32_t index) -> eastl::pair<uint32_t, bool>
            {
                auto itr = eastl::find(begin(colors), end(colors), index);
                if (itr != end(colors))
                {
                    return eastl::make_pair(uint32_t(itr - begin(colors)), false);
                }
                else
                {
                    uint32_t ret = colors.size();
                    colors.push_back(index);
                    return eastl::make_pair(ret, true);
                }
            };

            const auto addUniqueInputAttachment = [&](uint32_t index) -> eastl::pair<uint32_t, bool>
            {
                if (index == physicalPass.physicalDepthStencilAttachment)
                {
                    // the N + 1 attachment refers to depth
                    return eastl::make_pair(uint32_t(colors.size()), false);
                }
                else
                {
                    return addUniqueColor(index);
                }
            };

            for (auto& subpass : physicalPass.passes)
            {
                eastl::vector<ScaledClearRequests> scaledClearRequests;

                auto& pass = *m_passes[subpass];
                uint32_t subpassIndex = uint32_t(&subpass - physicalPass.passes.data());

                // add color attachments
                uint32_t numColorAttachments = pass.GetColorOutputs().size();
                physicalPass.subpasses[subpassIndex].numColorAttachments = numColorAttachments;

                for (uint32_t i = 0; i < numColorAttachments; i++)
                {
                    auto res = addUniqueColor(pass.GetColorOutputs()[i]->GetPhysicalIndex());
                    physicalPass.subpasses[subpassIndex].colorAttachments[i] = res.first;

                    if (res.second) // This is the first time the color attachment is used, check if we need load, or if we can clear it.
                    {
                        bool hasColorInput = !pass.GetColorInputs().empty() && pass.GetColorInputs()[i];
                        bool hasScaledColorInput = !pass.GetColorScaleInputs().empty() && pass.GetColorScaleInputs()[i];

                        if (!hasColorInput && !hasScaledColorInput)
                        {
                            if (pass.GetClearColor(i))
                            {
                                rp.clearAttachments |= 1u << res.first;
                                physicalPass.colorClearRequests.push_back({ &pass, &rp.clearColor[res.first], i });
                            }
                        }
                        else
                        {
                            if (hasScaledColorInput)
                            {
                                scaledClearRequests.push_back({ i, pass.GetColorScaleInputs()[i]->GetPhysicalIndex() });
                            }
                            else
                            {
                                rp.loadAttachments |= 1u << res.first;
                            }
                        }
                    }
                }

                if (!pass.GetResolveOutputs().empty())
                {
                    physicalPass.subpasses[subpassIndex].numResolveAttachments = numColorAttachments;
                    for (uint32_t i = 0; i < numColorAttachments; i++)
                    {
                        auto res = addUniqueColor(pass.GetResolveOutputs()[i]->GetPhysicalIndex());
                        physicalPass.subpasses[subpassIndex].resolveAttachments[i] = res.first;
                        // resolve attachments are don't care always
                    }
                }

                physicalPass.scaledClearRequests.push_back(eastl::move(scaledClearRequests));

                auto* dsInput = pass.GetDepthStencilInput();
                auto* dsOutput = pass.GetDepthStencilOutput();

                const auto addUniqueDS = [&](uint32_t index) -> eastl::pair<uint32_t, bool>
                {
                    assert(physicalPass.physicalDepthStencilAttachment == RenderResource::Unused || physicalPass.physicalDepthStencilAttachment == index);

                    bool isNewAttachment = physicalPass.physicalDepthStencilAttachment == RenderResource::Unused;
                    physicalPass.physicalDepthStencilAttachment = index;
                    return eastl::make_pair(index, isNewAttachment);
                };

                if (dsOutput && dsInput)
                {
                    auto res = addUniqueDS(dsOutput->GetPhysicalIndex());

                    // if this is the first subpass the attachment is used, we need to load it
                    if (res.second)
                    {
                        rp.loadAttachments |= 1u << res.first;
                    }

                    rp.opFlags |= RenderPassOp::StoreDepthStencil;
                    physicalPass.subpasses[subpassIndex].depthStencilMode = RenderPassInfo::DepthStencilMode::ReadWrite;
                }
                else if (dsOutput)
                {
                    auto res = addUniqueDS(dsOutput->GetPhysicalIndex());

                    // if this is the first subpass the attachment is used, we need to either clear or discard
                    if (res.second && pass.GetClearDepthStencil())
                    {
                        rp.opFlags |= RenderPassOp::ClearDepthStencil;
                        physicalPass.depthClearRequest.pass = &pass;
                        physicalPass.depthClearRequest.target = &rp.clearDepthStencil;
                    }

                    rp.opFlags |= RenderPassOp::StoreDepthStencil;
                    physicalPass.subpasses[subpassIndex].depthStencilMode = RenderPassInfo::DepthStencilMode::ReadWrite;

                    assert(physicalPass.physicalDepthStencilAttachment == RenderResource::Unused || physicalPass.physicalDepthStencilAttachment == dsOutput->GetPhysicalIndex());
                    physicalPass.physicalDepthStencilAttachment = dsOutput->GetPhysicalIndex();
                }
                else if (dsInput)
                {
                    auto res = addUniqueDS(dsInput->GetPhysicalIndex());

                    // if this is the first subpass the attachment is used, we need to load
                    if (res.second)
                    {
                        rp.opFlags |= RenderPassOp::DepthStencilReadOnly | RenderPassOp::LoadDepthStencil;

                        // check if any future passes need the depth information
                        bool preserveDepth = false;
                        for (auto& readPass : dsInput->GetReadPasses())
                        {
                            if (m_passes[readPass]->GetPhysicalPassIndex() > uint32_t(&physicalPass - m_physicalPasses.data()))
                            {
                                preserveDepth = true;
                                break;
                            }
                        }

                        if (preserveDepth)
                        {
                            // have to store here, or the attachment becomes undefined in future passes
                            rp.opFlags |= RenderPassOp::StoreDepthStencil;
                        }
                    }

                    physicalPass.subpasses[subpassIndex].depthStencilMode = RenderPassInfo::DepthStencilMode::ReadOnly;
                }
                else
                {
                    physicalPass.subpasses[subpassIndex].depthStencilMode = RenderPassInfo::DepthStencilMode::None;
                }
            }

            // add input attachments in a separate loop so we can pick up depth stencil input attachments properly
            for (auto& subpass : physicalPass.passes)
            {
                auto& pass = *m_passes[subpass];
                uint32_t subpassIndex = uint32_t(&subpass - physicalPass.passes.data());

                uint32_t numInputAttachments = pass.GetAttachmentInputs().size();
                physicalPass.subpasses[subpassIndex].numInputAttachments = numInputAttachments;

                for (uint32_t i = 0; i < numInputAttachments; i++)
                {
                    auto res = addUniqueInputAttachment(pass.GetAttachmentInputs()[i]->GetPhysicalIndex());
                    physicalPass.subpasses[subpassIndex].inputAttachments[i] = res.first;

                    // if this is the first subpass the attachment is used, we need to load it
                    if (res.second)
                    {
                        rp.loadAttachments |= 1u << res.first;
                    }
                }
            }

            physicalPass.renderPassInfo.numColorAttachments = physicalPass.physicalColorAttachments.size();
        }
    }

    void RenderGraph::BuildBarriers()
    {
        m_passBarriers.clear();
        m_passBarriers.reserve(m_passStack.size());

        const auto getAccess = [&](eastl::vector<Barrier>& barriers, uint32_t index, bool history) -> Barrier&
        {
            auto itr = eastl::find_if(begin(barriers), end(barriers), [index, history](const Barrier& b)
                {
                    return index == b.resourceIndex && history == b.history;
                });
            if (itr != end(barriers))
            {
                return *itr;
            }
            else
            {
                barriers.push_back({ index, VK_IMAGE_LAYOUT_UNDEFINED, 0, 0, history });
                return barriers.back();
            }
        };

        for (auto& index : m_passStack)
        {
            auto& pass = *m_passes[index];
            Barriers barriers;

            const auto getInvalidateAccess = [&](uint32_t i, bool history) -> Barrier&
            {
                return getAccess(barriers.invalidate, i, history);
            };

            const auto getFlushAccess = [&](uint32_t i) -> Barrier&
            {
                return getAccess(barriers.flush, i, false);
            };

            for (auto& input : pass.GetGenericBufferInputs())
            {
                auto& barrier = getInvalidateAccess(input.buffer->GetPhysicalIndex(), false);
                barrier.access |= input.access;
                barrier.stages |= input.stages;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for generic buffer input \"%s\"!", 
                        pass.GetName().c_str(),
                        input.buffer->GetName().c_str());
                }

                barrier.layout = input.layout;
            }

            for (auto& input : pass.GetGenericTextureInputs())
            {
                auto& barrier = getInvalidateAccess(input.texture->GetPhysicalIndex(), false);
                barrier.access |= input.access;
                barrier.stages |= input.stages;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for generic texture input \"%s\"!",
                        pass.GetName().c_str(),
                        input.texture->GetName().c_str());
                }

                barrier.layout = input.layout;
            }

            for (auto* input : pass.GetHistoryInputs())
            {
                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), true);
                barrier.access |= VK_ACCESS_SHADER_READ_BIT;

                if (HAS_NO_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: Pick appropriate stage.
                }
                else
                {
                    barrier.stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for history input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            for (auto* input : pass.GetAttachmentInputs())
            {
                if (HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" is compute but has an input attachment (\"%s\")!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for attachment input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            for (auto* input : pass.GetStorageInputs())
            {
                if (!input)
                {
                    continue;
                }

                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

                if (HAS_NO_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: Pick appropriate stage.
                }
                else
                {
                    barrier.stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for storage input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_GENERAL;
            }

            for (auto* input : pass.GetStorageTextureInputs())
            {
                if (!input)
                {
                    continue;
                }

                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

                if (HAS_NO_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: Pick appropriate stage.
                }
                else
                {
                    barrier.stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for storage texture input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_GENERAL;
            }

            for (auto* input : pass.GetBlitTextureInputs())
            {
                if (!input)
                {
                    continue;
                }

                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for blit texture input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            }

            for (auto* input : pass.GetColorInputs())
            {
                if (!input)
                {
                    continue;
                }

                if (HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" is compute but has a color input (\"%s\")!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                barrier.stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                // if the attachment is also bound as an input attachment (programmable blending) we need VK_IMAGE_LAYOUT_GENERAL
                if (barrier.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.layout = VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for color input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }
                else
                {
                    barrier.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
            }

            for (auto* input : pass.GetColorScaleInputs())
            {
                if (!input)
                {
                    continue;
                }

                if (HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" is compute but has a scaled color input (\"%s\")!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                auto& barrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_SHADER_READ_BIT;
                barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for scaled color input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            for (auto* output : pass.GetColorOutputs())
            {
                if (HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" is compute but has a color output (\"%s\")!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }

                auto& barrier = getFlushAccess(output->GetPhysicalIndex());

                if (m_physicalDimensions[output->GetPhysicalIndex()].levels > 1)
                {
                    // access should be 0 here. generate_mipmaps will take care of invalidation needed
                    barrier.access |= VK_ACCESS_TRANSFER_READ_BIT; // validation layers complain without this
                    barrier.stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;

                    if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Layout mismatch in pass \"%s\" for color output \"%s\"!",
                            pass.GetName().c_str(),
                            output->GetName().c_str());
                    }

                    barrier.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                }
                else
                {
                    barrier.access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    barrier.stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                    // if the attachment is also bound as an input attachment (programmable blending) we need VK_IMAGE_LAYOUT_GENERAL
                    if (barrier.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL || barrier.layout == VK_IMAGE_LAYOUT_GENERAL)
                    {
                        barrier.layout = VK_IMAGE_LAYOUT_GENERAL;
                    }
                    else if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        Logger::ErrorTF(LOG_TAG,
                            "Layout mismatch in pass \"%s\" for color output \"%s\"!",
                            pass.GetName().c_str(),
                            output->GetName().c_str());
                    }
                    else
                    {
                        barrier.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    }
                }
            }

            for (auto* output : pass.GetResolveOutputs())
            {
                if (HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Pass \"%s\" is compute but has a resolve output (\"%s\")!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }

                auto& barrier = getFlushAccess(output->GetPhysicalIndex());
                barrier.access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for resolve output \"%s\"!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            for (auto* output : pass.GetBlitTextureOutputs())
            {
                auto& barrier = getInvalidateAccess(output->GetPhysicalIndex(), false);
                barrier.access |= VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for blit texture output \"%s\"!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            }

            for (auto* output : pass.GetStorageOutputs())
            {
                auto& barrier = getFlushAccess(output->GetPhysicalIndex());
                barrier.access |= VK_ACCESS_SHADER_WRITE_BIT;

                if (HAS_NO_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: Pick appropriate stage.
                }
                else
                {
                    barrier.stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for storage output \"%s\"!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_GENERAL;
            }

            for (auto* output : pass.GetStorageTextureOutputs())
            {
                auto& barrier = getFlushAccess(output->GetPhysicalIndex());
                barrier.access |= VK_ACCESS_SHADER_WRITE_BIT;

                if (HAS_NO_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
                {
                    barrier.stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // TODO: Pick appropriate stage.
                }
                else
                {
                    barrier.stages |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                }

                if (barrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for storage texture output \"%s\"!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }

                barrier.layout = VK_IMAGE_LAYOUT_GENERAL;
            }

            auto* output = pass.GetDepthStencilOutput();
            auto* input = pass.GetDepthStencilInput();

            if (input && HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" is compute but has a depth stencil input (\"%s\")!",
                    pass.GetName().c_str(),
                    input->GetName().c_str());
            }
            if (output && HAS_ANY_FLAG(pass.GetQueue(), COMPUTE_QUEUES))
            {
                Logger::ErrorTF(LOG_TAG,
                    "Pass \"%s\" is compute but has a depth stencil output (\"%s\")!",
                    pass.GetName().c_str(),
                    output->GetName().c_str());
            }

            if (output && input)
            {
                auto& dstBarrier = getInvalidateAccess(input->GetPhysicalIndex(), false);
                auto& srcBarrier = getFlushAccess(output->GetPhysicalIndex());

                if (dstBarrier.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    dstBarrier.layout = VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (dstBarrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for depth stencil input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }
                else
                {
                    dstBarrier.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }

                dstBarrier.access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dstBarrier.stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

                srcBarrier.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                srcBarrier.access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                srcBarrier.stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }
            else if (input)
            {
                auto& dstBarrier = getInvalidateAccess(input->GetPhysicalIndex(), false);

                if (dstBarrier.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    dstBarrier.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                }
                else if (dstBarrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for depth stencil input \"%s\"!",
                        pass.GetName().c_str(),
                        input->GetName().c_str());
                }
                else
                {
                    dstBarrier.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                }

                dstBarrier.access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                dstBarrier.stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }
            else if (output)
            {
                auto& srcBarrier = getFlushAccess(output->GetPhysicalIndex());

                if (srcBarrier.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    srcBarrier.layout = VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (srcBarrier.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    Logger::ErrorTF(LOG_TAG,
                        "Layout mismatch in pass \"%s\" for depth stencil output \"%s\"!",
                        pass.GetName().c_str(),
                        output->GetName().c_str());
                }
                else
                {
                    srcBarrier.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }

                srcBarrier.access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                srcBarrier.stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }

            m_passBarriers.push_back(eastl::move(barriers));
        }
    }

    void RenderGraph::BuildPhysicalBarriers()
    {
        auto barrierItr = begin(m_passBarriers);

        const auto flushAccessToInvalidate = [](VkAccessFlags flags) -> VkAccessFlags
        {
            if (flags & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            {
                flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            }
            if (flags & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
            {
                flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            }
            if (flags & VK_ACCESS_SHADER_WRITE_BIT)
            {
                flags |= VK_ACCESS_SHADER_READ_BIT;
            }
            return flags;
        };

        struct ResourceState
        {
            VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkAccessFlags invalidatedTypes = 0;
            VkAccessFlags flushedTypes = 0;

            VkPipelineStageFlags invalidatedStages = 0;
            VkPipelineStageFlags flushedStages = 0;
        };

        // to handle state inside a physical pass
        eastl::vector<ResourceState> resourceState;
        resourceState.reserve(m_physicalDimensions.size());

        for (auto& physicalPass : m_physicalPasses)
        {
            resourceState.clear();
            resourceState.resize(m_physicalDimensions.size());

            // Go over all physical passes, and observe their use of barriers.
            // In multipass, only the first and last barriers need to be considered externally.
            // Compute never has multipass.
            uint32_t subpasses = physicalPass.passes.size();
            for (uint32_t i = 0; i < subpasses; i++, ++barrierItr)
            {
                auto& barriers = *barrierItr;
                auto& invalidates = barriers.invalidate;
                auto& flushes = barriers.flush;

                for (auto& invalidate : invalidates)
                {
                    auto& res = resourceState[invalidate.resourceIndex];

                    // transients and swapchain images are handled implicitly
                    if (m_physicalDimensions[invalidate.resourceIndex].transient ||
                        invalidate.resourceIndex == m_swapchainPhysicalIndex)
                    {
                        continue;
                    }

                    if (invalidate.history)
                    {
                        auto itr = eastl::find_if(begin(physicalPass.invalidate), end(physicalPass.invalidate), [&](const Barrier& b) -> bool
                            {
                                return b.resourceIndex == invalidate.resourceIndex && b.history;
                            });

                        if (itr == end(physicalPass.invalidate))
                        {
                            // storage images should just be in GENERAL all the time instead of SHADER_READ_ONLY_OPTIMAL
                            auto layout = m_physicalDimensions[invalidate.resourceIndex].IsStorageImage() ?
                                VK_IMAGE_LAYOUT_GENERAL :
                                invalidate.layout;

                            // Special case history barriers. They are a bit different from other barriers.
                            // We just need to ensure the layout is right and that we avoid write-after-read.
                            // Even if we see these barriers in multiple render passes, they will not emit multiple barriers.
                            physicalPass.invalidate.push_back({ invalidate.resourceIndex, layout, invalidate.access, invalidate.stages, true });
                            physicalPass.flush.push_back({ invalidate.resourceIndex, layout, 0, invalidate.stages, true });
                        }

                        continue;
                    }

                    // only the first use of a resource in a physical pass needs to be handled externally
                    if (res.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        res.invalidatedTypes |= invalidate.access;
                        res.invalidatedStages |= invalidate.stages;

                        // storage images should just be in GENERAL all the time instead of SHADER_READ_ONLY_OPTIMAL
                        if (m_physicalDimensions[invalidate.resourceIndex].IsStorageImage())
                        {
                            res.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                        }
                        else
                        {
                            res.initialLayout = invalidate.layout;
                        }
                    }

                    // a read-only invalidation can change the layout
                    if (m_physicalDimensions[invalidate.resourceIndex].IsStorageImage())
                    {
                        res.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }
                    else
                    {
                        res.finalLayout = invalidate.layout;
                    }

                    // All pending flushes have been invalidated in the appropriate stages already.
                    // This is relevant if the invalidate happens in subpass #1 and beyond.
                    res.flushedTypes = 0;
                    res.flushedStages = 0;
                }

                for (auto& flush : flushes)
                {
                    auto& res = resourceState[flush.resourceIndex];

                    // transients are handled implicitly
                    if (m_physicalDimensions[flush.resourceIndex].transient || flush.resourceIndex == m_swapchainPhysicalIndex)
                    {
                        continue;
                    }

                    // the last use of a resource in a physical pass needs to be handled externally
                    res.flushedTypes |= flush.access;
                    res.flushedStages |= flush.stages;

                    // storage images should just be in GENERAL all the time instead of SHADER_READ_ONLY_OPTIMAL
                    if (m_physicalDimensions[flush.resourceIndex].IsStorageImage())
                    {
                        res.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                    }
                    else
                    {
                        res.finalLayout = flush.layout;
                    }

                    // If we didn't have an invalidation before first flush, we must invalidate first.
                    // Only first flush in a render pass needs a matching invalidation.
                    if (res.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        // If we end in TRANSFER_SRC_OPTIMAL, we actually start in COLOR_ATTACHMENT_OPTIMAL.
                        if (flush.layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                        {
                            res.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            res.invalidatedStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                            res.invalidatedTypes = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                        }
                        else
                        {
                            res.initialLayout = flush.layout;
                            res.invalidatedStages = flush.stages;
                            res.invalidatedTypes = flushAccessToInvalidate(flush.access);
                        }

                        // We're not reading the resource in this pass, so we might as well transition from UNDEFINED to discard the resource.
                        physicalPass.discards.push_back(flush.resourceIndex);
                    }
                }
            }

            // Now that the render pass has been studied, look at each resource individually and see how we need to deal
            // with the physical render pass as a whole.
            for (auto& resource : resourceState)
            {
                // resource was not touched in this pass
                if (resource.finalLayout == VK_IMAGE_LAYOUT_UNDEFINED && resource.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    continue;
                }

                assert(resource.finalLayout != VK_IMAGE_LAYOUT_UNDEFINED);

                uint32_t index = uint32_t(&resource - resourceState.data());

                physicalPass.invalidate.push_back({ index, resource.initialLayout, resource.invalidatedTypes, resource.invalidatedStages, false });

                if (resource.flushedTypes)
                {
                    // Did the pass write anything in this pass which needs to be flushed?
                    physicalPass.flush.push_back({ index, resource.finalLayout, resource.flushedTypes, resource.flushedStages, false });
                }
                else if (resource.invalidatedTypes)
                {
                    // Did the pass read anything in this pass which needs to be protected before it can be written?
                    // Implement this as a flush with 0 access bits.
                    // This is how Vulkan essentially implements a write-after-read hazard.
                    // The only purpose of this flush barrier is to set the last pass which the resource was used as a stage.
                    // Do not clear lastInvalidatePass, because we can still keep tacking on new access flags, etc.
                    physicalPass.flush.push_back({ index, resource.finalLayout, 0, resource.invalidatedStages, false });
                }

                // If we end in TRANSFER_SRC_OPTIMAL, this is a sentinel for needing mipmapping, so enqueue that up here.
                if (resource.finalLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                {
                    physicalPass.mipmapRequests.push_back({ index, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
                }
            }
        }
    }

    void RenderGraph::BuildAliases()
    {
        struct Range
        {
            uint32_t firstWritePass = ~0u;
            uint32_t firstReadPass = ~0u;
            uint32_t lastWritePass = 0;
            uint32_t lastReadPass = 0;
            bool blockAlias = false;

            bool HasWriter() const
            {
                return firstWritePass <= lastWritePass;
            }

            bool HasReader() const
            {
                return firstReadPass <= lastReadPass;
            }

            bool IsUsed() const
            {
                return HasWriter() || HasReader();
            }

            bool CanAlias() const
            {
                // if we read before we have completely written to a resource we need to preserve it, so no alias is possible
                if (HasReader() && HasWriter() && firstReadPass <= firstWritePass)
                {
                    return false;
                }
                if (blockAlias)
                {
                    return false;
                }
                return true;
            }

            uint32_t FirstUsedPass() const
            {
                uint32_t firstPass = ~0u;
                if (HasWriter())
                {
                    firstPass = eastl::min(firstPass, firstWritePass);
                }
                if (HasReader())
                {
                    firstPass = eastl::min(firstPass, firstReadPass);
                }
                return firstPass;
            }

            uint32_t LastUsedPass() const
            {
                uint32_t lastPass = 0;
                if (HasWriter())
                {
                    lastPass = eastl::max(lastPass, lastWritePass);
                }
                if (HasReader())
                {
                    lastPass = eastl::max(lastPass, lastReadPass);
                }
                return lastPass;
            }

            bool DisjointLifetime(const Range& range) const
            {
                if (!IsUsed() || !range.IsUsed())
                {
                    return false;
                }
                if (!CanAlias() || !range.CanAlias())
                {
                    return false;
                }

                bool left = LastUsedPass() < range.FirstUsedPass();
                bool right = range.LastUsedPass() < FirstUsedPass();
                return left || right;
            }
        };

        eastl::vector<Range> passRange(m_physicalDimensions.size());

        const auto registerReader = [&passRange](const RenderTextureResource* resource, uint32_t passIndex)
        {
            if (resource && passIndex != RenderPass::Unused)
            {
                uint32_t phys = resource->GetPhysicalIndex();
                if (phys != RenderResource::Unused)
                {
                    auto& range = passRange[phys];
                    range.lastReadPass = eastl::max(range.lastReadPass, passIndex);
                    range.firstReadPass = eastl::min(range.firstReadPass, passIndex);
                }
            }
        };

        const auto registerWriter = [&passRange](const RenderTextureResource* resource, uint32_t passIndex, bool blockAlias)
        {
            if (resource && passIndex != RenderPass::Unused)
            {
                uint32_t phys = resource->GetPhysicalIndex();
                if (phys != RenderResource::Unused)
                {
                    auto& range = passRange[phys];
                    range.lastWritePass = eastl::max(range.lastWritePass, passIndex);
                    range.firstWritePass = eastl::min(range.firstWritePass, passIndex);
                    if (blockAlias)
                    {
                        range.blockAlias = blockAlias;
                    }
                }
            }
        };

        for (auto& pass : m_passStack)
        {
            auto& subpass = *m_passes[pass];

            for (auto* input : subpass.GetColorInputs())
            {
                registerReader(input, subpass.GetPhysicalPassIndex());
            }
            for (auto* input : subpass.GetColorScaleInputs())
            {
                registerReader(input, subpass.GetPhysicalPassIndex());
            }
            for (auto* input : subpass.GetAttachmentInputs())
            {
                registerReader(input, subpass.GetPhysicalPassIndex());
            }
            for (auto& input : subpass.GetGenericTextureInputs())
            {
                registerReader(input.texture, subpass.GetPhysicalPassIndex());
            }
            for (auto* input : subpass.GetBlitTextureInputs())
            {
                registerReader(input, subpass.GetPhysicalPassIndex());
            }
            for (auto* input : subpass.GetStorageTextureInputs())
            {
                registerReader(input, subpass.GetPhysicalPassIndex());
            }
            if (subpass.GetDepthStencilInput())
            {
                registerReader(subpass.GetDepthStencilInput(), subpass.GetPhysicalPassIndex());
            }

            // if a subpass may not execute, we cannot alias with that resource because some other pass may invalidate it
            bool block_alias = subpass.MayNotNeedRenderPass();

            if (subpass.GetDepthStencilOutput())
            {
                registerWriter(subpass.GetDepthStencilOutput(), subpass.GetPhysicalPassIndex(), block_alias);
            }
            for (auto* output : subpass.GetColorOutputs())
            {
                registerWriter(output, subpass.GetPhysicalPassIndex(), block_alias);
            }
            for (auto* output : subpass.GetResolveOutputs())
            {
                registerWriter(output, subpass.GetPhysicalPassIndex(), block_alias);
            }
            for (auto* output : subpass.GetBlitTextureOutputs())
            {
                registerWriter(output, subpass.GetPhysicalPassIndex(), block_alias);
            }

            // storage textures are not aliased, because they are implicitly preserved
            for (auto* output : subpass.GetStorageTextureOutputs())
            {
                registerWriter(output, subpass.GetPhysicalPassIndex(), true);
            }
        }

        eastl::vector<eastl::vector<uint32_t>> aliasChains(m_physicalDimensions.size());

        m_physicalAliases.resize(m_physicalDimensions.size());
        for (auto& v : m_physicalAliases)
        {
            v = RenderResource::Unused;
        }

        for (uint32_t i = 0; i < m_physicalDimensions.size(); i++)
        {
            // no aliases for buffers
            if (m_physicalDimensions[i].bufferInfo.size)
            {
                continue;
            }

            // No aliases for images with history
            if (m_physicalImageHasHistory[i])
            {
                continue;
            }

            // only try to alias with lower-indexed resources, because we allocate them one-by-one starting from index 0
            for (uint32_t j = 0; j < i; j++)
            {
                if (m_physicalImageHasHistory[j])
                {
                    continue;
                }

                if (m_physicalDimensions[i] == m_physicalDimensions[j])
                {
                    // Only alias if the resources are used in the same queue, this way we avoid introducing
                    // multi-queue shenanigans. We can only use events to pass aliasing barriers.
                    // Also, only alias if we have one single queue.
                    bool sameSingleQueue = m_physicalDimensions[i].queues == m_physicalDimensions[j].queues;

                    if (HAS_ANY_FLAG(m_physicalDimensions[i].queues, m_physicalDimensions[i].queues - 1))
                    {
                        sameSingleQueue = false;
                    }

                    if (passRange[i].DisjointLifetime(passRange[j]) && sameSingleQueue)
                    {
                        // we can alias
                        m_physicalAliases[i] = j;
                        if (aliasChains[j].empty())
                        {
                            aliasChains[j].push_back(j);
                        }
                        aliasChains[j].push_back(i);

                        // We might have different image usage, propagate this information.
                        auto mergedImageUsage = m_physicalDimensions[j].imageUsage |= m_physicalDimensions[i].imageUsage;
                        m_physicalDimensions[i].imageUsage = mergedImageUsage;
                        m_physicalDimensions[j].imageUsage = mergedImageUsage;
                        break;
                    }
                }
            }
        }

        // now we've found the aliases, so set up the transfer barriers in order of use
        for (auto& chain : aliasChains)
        {
            if (chain.empty())
            {
                continue;
            }

            eastl::sort(begin(chain), end(chain), [&](uint32_t a, uint32_t b) -> bool
                {
                    return passRange[a].LastUsedPass() < passRange[b].FirstUsedPass();
                });

            for (uint32_t i = 0; i < chain.size(); i++)
            {
                if (i + 1 < chain.size())
                {
                    m_physicalPasses[passRange[chain[i]].LastUsedPass()].aliasTransfer.push_back(eastl::make_pair(chain[i], chain[i + 1]));
                }
                else
                {
                    m_physicalPasses[passRange[chain[i]].LastUsedPass()].aliasTransfer.push_back(eastl::make_pair(chain[i], chain[0]));
                }
            }
        }
    }

    void RenderGraph::Log()
    {
        Logger::DebugT(LOG_TAG, "------------------------RENDER GRAPH START------------------------");

        for (auto& resource : m_physicalDimensions)
        {
            if (resource.bufferInfo.size)
            {
                Logger::DebugTF(LOG_TAG, "Resource #%u (\"%s\"): size: %u",
                    uint32_t(&resource - m_physicalDimensions.data()),
                    resource.name.c_str(),
                    uint32_t(resource.bufferInfo.size));
            }
            else
            {
                Logger::DebugTF(LOG_TAG, "Resource #%u (\"%s\"): %u x %u, format: %s, samples: %u, transient: %s%s",
                    uint32_t(&resource - m_physicalDimensions.data()),
                    resource.name.c_str(),
                    resource.width, resource.height,
                    FormatToString(resource.format).c_str(),
                    resource.samples,
                    resource.transient ? "yes" : "no",
                    uint32_t(&resource - m_physicalDimensions.data()) == m_swapchainPhysicalIndex ? " (swapchain)" : "");
            }
        }

        auto barrierItr = begin(m_passBarriers);

        const auto swapStr = [this](const Barrier& barrier) -> const char*
        {
            return barrier.resourceIndex == m_swapchainPhysicalIndex ? " (swapchain)" : "";
        };

        for (auto& subpasses : m_physicalPasses)
        {
            Logger::DebugTF(LOG_TAG, "Pass #%u:", uint32_t(&subpasses - m_physicalPasses.data()));

            for (auto& barrier : subpasses.invalidate)
            {
                Logger::DebugTF(LOG_TAG, "  Invalidate: %u%s, layout: %s, access: %s, stages: %s",
                    barrier.resourceIndex,
                    swapStr(barrier),
                    LayoutToString(barrier.layout).c_str(),
                    AccessFlagsToString(barrier.access).c_str(),
                    StageFlagsToString(barrier.stages).c_str());
            }

            for (auto& subpass : subpasses.passes)
            {
                Logger::DebugTF(LOG_TAG, "    Subpass #%u (%s):", uint32_t(&subpass - subpasses.passes.data()), m_passes[subpass]->GetName().c_str());
                auto& pass = *m_passes[subpass];

                auto& barriers = *barrierItr;
                for (auto& barrier : barriers.invalidate)
                {
                    if (!m_physicalDimensions[barrier.resourceIndex].transient)
                    {
                        Logger::DebugTF(LOG_TAG, "      Invalidate: %u%s, layout: %s, access: %s, stages: %s",
                            barrier.resourceIndex,
                            swapStr(barrier),
                            LayoutToString(barrier.layout).c_str(),
                            AccessFlagsToString(barrier.access).c_str(),
                            StageFlagsToString(barrier.stages).c_str());
                    }
                }

                if (pass.GetDepthStencilOutput())
                {
                    Logger::DebugTF(LOG_TAG, "        DepthStencil ReadWrite: %u", pass.GetDepthStencilOutput()->GetPhysicalIndex());
                }
                else if (pass.GetDepthStencilInput())
                {
                    Logger::DebugTF(LOG_TAG, "        DepthStencil ReadOnly: %u", pass.GetDepthStencilInput()->GetPhysicalIndex());
                }

                for (auto& output : pass.GetColorOutputs())
                {
                    Logger::DebugTF(LOG_TAG, "        ColorAttachment #%u: %u", uint32_t(&output - pass.GetColorOutputs().data()), output->GetPhysicalIndex());
                }
                for (auto& output : pass.GetResolveOutputs())
                {
                    Logger::DebugTF(LOG_TAG, "        ResolveAttachment #%u: %u", uint32_t(&output - pass.GetResolveOutputs().data()), output->GetPhysicalIndex());
                }
                for (auto& input : pass.GetAttachmentInputs())
                {
                    Logger::DebugTF(LOG_TAG, "        InputAttachment #%u: %u", uint32_t(&input - pass.GetAttachmentInputs().data()), input->GetPhysicalIndex());
                }
                for (auto& input : pass.GetGenericTextureInputs())
                {
                    Logger::DebugTF(LOG_TAG, "        Read-only texture #%u: %u", uint32_t(&input - pass.GetGenericTextureInputs().data()), input.texture->GetPhysicalIndex());
                }
                for (auto& input : pass.GetGenericBufferInputs())
                {
                    Logger::DebugTF(LOG_TAG, "        Read-only buffer #%u: %u", uint32_t(&input - pass.GetGenericBufferInputs().data()), input.buffer->GetPhysicalIndex());
                }

                for (auto& input : pass.GetColorScaleInputs())
                {
                    if (input)
                    {
                        Logger::DebugTF(LOG_TAG, "        ColorScaleInput #%u: %u",
                            uint32_t(&input - pass.GetColorScaleInputs().data()),
                            input->GetPhysicalIndex());
                    }
                }

                for (auto& barrier : barriers.flush)
                {
                    if (!m_physicalDimensions[barrier.resourceIndex].transient && barrier.resourceIndex != m_swapchainPhysicalIndex)
                    {
                        Logger::DebugTF(LOG_TAG, "      Flush: %u, layout: %s, access: %s, stages: %s",
                            barrier.resourceIndex,
                            LayoutToString(barrier.layout).c_str(),
                            AccessFlagsToString(barrier.access).c_str(),
                            StageFlagsToString(barrier.stages).c_str());
                    }
                }

                ++barrierItr;
            }

            for (auto& barrier : subpasses.flush)
            {
                Logger::DebugTF(LOG_TAG, "  Flush: %u%s, layout: %s, access: %s, stages: %s",
                    barrier.resourceIndex,
                    swapStr(barrier),
                    LayoutToString(barrier.layout).c_str(),
                    AccessFlagsToString(barrier.access).c_str(),
                    StageFlagsToString(barrier.stages).c_str());
            }
        }

        Logger::DebugT(LOG_TAG, "------------------------RENDER GRAPH END------------------------");
    }

    ResourceDimensions RenderGraph::GetResourceDimensions(const RenderBufferResource& resource) const
    {
        ResourceDimensions dim;
        auto& info = resource.GetBufferInfo();
        dim.bufferInfo = info;
        dim.bufferInfo.usage |= resource.GetBufferUsage();
        dim.persistent = info.persistent;
        dim.name = resource.GetName();
        return dim;
    }

    ResourceDimensions RenderGraph::GetResourceDimensions(const RenderTextureResource& resource) const
    {
        ResourceDimensions dim;
        auto& info = resource.GetAttachmentInfo();
        dim.layers = info.layers;
        dim.samples = info.samples;
        dim.format = info.format;
        dim.transient = resource.GetTransientState();
        dim.persistent = info.persistent;
        dim.unormSrgb = info.aliasUnormSrgb;
        dim.queues = resource.GetUsedQueues();
        dim.imageUsage = info.auxUsage | resource.GetImageUsage();
        dim.name = resource.GetName();

        switch (info.sizeMode)
        {
            case SizeMode::Absolute:
                dim.width   = eastl::max(uint32_t(info.sizeX), 1u);
                dim.height  = eastl::max(uint32_t(info.sizeY), 1u);
                dim.depth   = eastl::max(uint32_t(info.sizeZ), 1u);
                break;

            case SizeMode::SwapchainRelative:
                dim.width   = eastl::max(uint32_t(std::ceil(info.sizeX * m_swapchainDimensions.width)), 1u);
                dim.height  = eastl::max(uint32_t(std::ceil(info.sizeY * m_swapchainDimensions.height)), 1u);
                dim.depth   = eastl::max(uint32_t(std::ceil(info.sizeZ)), 1u);
                break;

            case SizeMode::InputRelative:
            {
                auto itr = m_resourceToIndex.find(info.sizeRelativeName);
                if (itr == end(m_resourceToIndex))
                {
                    Logger::ErrorTF(LOG_TAG, "Size relative input resource for \"%s\" does not exist!", dim.name.c_str());
                }
                auto& input = static_cast<RenderTextureResource&>(*m_resources[itr->second]);
                auto inputDim = GetResourceDimensions(input);

                dim.width   = eastl::max(uint32_t(std::ceil(inputDim.width * info.sizeX)), 1u);
                dim.height  = eastl::max(uint32_t(std::ceil(inputDim.height * info.sizeY)), 1u);
                dim.depth   = eastl::max(uint32_t(std::ceil(inputDim.depth * info.sizeZ)), 1u);
                break;
            }
        }

        if (dim.format == VK_FORMAT_UNDEFINED)
        {
            dim.format = m_swapchainDimensions.format;
        }

        dim.levels = eastl::min(Image::NumMipLevels({ dim.width, dim.height, dim.depth }), info.levels == 0 ? ~0u : info.levels);

        return dim;
    }

    void RenderGraph::SetupPhysicalBuffer(uint32_t attachment)
    {
        auto& att = m_physicalDimensions[attachment];

        bool needBuffer = true;
        if (m_physicalBuffers[attachment])
        {
            if (att.persistent &&
                m_physicalBuffers[attachment]->GetSize() == att.bufferInfo.size &&
                HAS_FLAGS(m_physicalBuffers[attachment]->GetUsage(), att.bufferInfo.usage))
            {
                needBuffer = false;
            }
        }

        if (needBuffer)
        {
            auto buffer = eastl::make_shared<Buffer>(att.bufferInfo.size, att.bufferInfo.usage, VMA_MEMORY_USAGE_GPU_ONLY);
            buffer->SetName(att.name.c_str());

            m_physicalBuffers[attachment] = buffer;
            m_physicalEvents[attachment] = {};
        }
    }

    void RenderGraph::SetupPhysicalImage(uint32_t attachment)
    {
        auto& att = m_physicalDimensions[attachment];

        if (m_physicalAliases[attachment] != RenderResource::Unused)
        {
            m_physicalImageAttachments[attachment] = m_physicalImageAttachments[m_physicalAliases[attachment]];
            m_physicalAttachments[attachment] = &m_physicalImageAttachments[attachment]->get_view();
            m_physicalEvents[attachment] = {};
            return;
        }

        bool needImage = true;
        VkImageUsageFlags usage = att.imageUsage;
        Vulkan::ImageMiscFlags misc = 0;
        VkImageCreateFlags flags = 0;

        if (att.unormSrgb)
        {
            misc |= Vulkan::IMAGE_MISC_MUTABLE_SRGB_BIT;
        }
        if (att.IsStorageImage())
        {
            flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        }

        if (m_physicalImageAttachments[attachment])
        {
            if (att.persistent &&
                m_physicalImageAttachments[attachment]->get_create_info().format == att.format &&
                m_physicalImageAttachments[attachment]->get_create_info().width == att.width &&
                m_physicalImageAttachments[attachment]->get_create_info().height == att.height &&
                m_physicalImageAttachments[attachment]->get_create_info().depth == att.depth &&
                m_physicalImageAttachments[attachment]->get_create_info().samples == att.samples &&
                HAS_FLAGS(m_physicalImageAttachments[attachment]->GetUsage(), usage) &&
                HAS_FLAGS(m_physicalImageAttachments[attachment]->get_create_info().flags, flags))
            {
                needImage = false;
            }
        }

        if (needImage)
        {
            Vulkan::ImageCreateInfo info;
            info.format = att.format;
            info.type = att.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
            info.width = att.width;
            info.height = att.height;
            info.depth = att.depth;
            info.domain = Vulkan::ImageDomain::Physical;
            info.levels = att.levels;
            info.layers = att.layers;
            info.usage = usage;
            info.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.samples = static_cast<VkSampleCountFlagBits>(att.samples);
            info.flags = flags;

            if (Vulkan::format_has_depth_or_stencil_aspect(info.format))
                info.usage &= ~VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            info.misc = misc;
            if (HAS_ANY_FLAG(att.queues, RenderGraphQueue::Graphics | RenderGraphQueue::Compute))
            {
                info.misc |= Vulkan::IMAGE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT;
            }
            if (HAS_ANY_FLAG(att.queues, RenderGraphQueue::AsyncCompute))
            {
                info.misc |= Vulkan::IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT;
            }
            if (HAS_ANY_FLAG(att.queues, RenderGraphQueue::AsyncGraphics))
            {
                info.misc |= Vulkan::IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_GRAPHICS_BIT;
            }

            //physical_image_attachments[attachment] = device_.create_image(info, nullptr);
            
            auto image = eastl::make_shared<Image>();
            image->SetDebugName(att.name.c_str());

            // Just keep storage images in GENERAL layout.
            // There is no reason to try enabling compression.
            if (!physical_image_attachments[attachment])
                LOGE("Failed to create render graph image!\n");
            if (att.is_storage_image())
                physical_image_attachments[attachment]->set_layout(Vulkan::Layout::General);
            physical_events[attachment] = {};
        }

        physical_attachments[attachment] = &physical_image_attachments[attachment]->get_view();
    }

    void RenderGraph::EnqueueMipmapRequests(CommandBuffer& cmd, const eastl::vector<MipmapRequests>& requests)
    {
        if (requests.empty())
        {
            return;
        }

        for (auto& req : requests)
        {
            auto& image = m_physicalAttachments[req.physical_resource]->get_image();

            cmd.begin_region("render-graph-mipgen");
            cmd.barrier_prepare_generate_mipmap(image, req.layout, req.stages, req.access);

            cmd.generate_mipmap(image);
            cmd.end_region();
        }
    }

    void RenderGraph::EnqueueScaledRequests(CommandBuffer& cmd, const eastl::vector<ScaledClearRequests>& requests)
    {
        if (requests.empty())
        {
            return;
        }

        eastl::vector<eastl::pair<String, int>> defines;
        defines.reserve(requests.size());

        for (auto& req : requests)
        {
            defines.push_back({ string("HAVE_TARGET_") + to_string(req.target), 1 });
            cmd.set_texture(0, req.target, *physical_attachments[req.physical_resource], Vulkan::StockSampler::LinearClamp);
        }

        Vulkan::CommandBufferUtil::draw_fullscreen_quad(cmd, "builtin://shaders/quad.vert", "builtin://shaders/scaled_readback.frag", defines);
    }
}
