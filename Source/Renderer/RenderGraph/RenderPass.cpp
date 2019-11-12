#include "stdafx.h"
#include "RenderPass.h"

#include "RenderGraph.h"

#define LOG_TAG MANTIS_TEXT("RenderPass")

namespace Mantis
{
    static const RenderGraphQueue COMPUTE_QUEUES = RenderGraphQueue::AsyncCompute | RenderGraphQueue::Compute;

    RenderTextureResource& RenderPass::SetDepthStencilInput(const String& name)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.ReadInPass(m_index);
        res.AddImageUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        m_depthStencilInput = &res;
        return res;
    }

    RenderTextureResource& RenderPass::AddAttachmentInput(const String& name)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.ReadInPass(m_index);
        res.AddImageUsage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        m_attachmentInputs.push_back(&res);
        return res;
    }

    RenderTextureResource& RenderPass::AddHistoryInput(const String& name)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.AddImageUsage(VK_IMAGE_USAGE_SAMPLED_BIT);
        m_historyInputs.push_back(&res);
        return res;
    }

    RenderBufferResource& RenderPass::AddGenericBufferInput(
        const String& name,
        VkPipelineStageFlags stages,
        VkAccessFlags access,
        VkBufferUsageFlags usage)
    {
        auto& res = m_graph.GetBufferResource(name);
        res.AddQueue(m_queue);
        res.ReadInPass(m_index);
        res.AddBufferUsage(usage);

        AccessedBufferResource acc;
        acc.buffer = &res;
        acc.layout = VK_IMAGE_LAYOUT_GENERAL;
        acc.access = access;
        acc.stages = stages;

        m_genericBuffers.push_back(acc);
        return res;
    }

    RenderBufferResource& RenderPass::AddVertexBufferInput(const String& name)
    {
        return AddGenericBufferInput(name,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    RenderBufferResource& RenderPass::AddIndexBufferInput(const String& name)
    {
        return AddGenericBufferInput(name,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            VK_ACCESS_INDEX_READ_BIT,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    RenderBufferResource& RenderPass::AddIndirectBufferInput(const String& name)
    {
        return AddGenericBufferInput(name,
            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
            VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    }

    RenderBufferResource& RenderPass::AddUniformInput(const String& name, VkPipelineStageFlags stages)
    {
        if (stages == 0)
        {
            if (HAS_ANY_FLAG(m_queue, COMPUTE_QUEUES))
            {
                stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            else
            {
                stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
        }
        return AddGenericBufferInput(name, stages, VK_ACCESS_UNIFORM_READ_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }

    RenderBufferResource& RenderPass::AddStorageReadOnlyInput(const String& name, VkPipelineStageFlags stages)
    {
        if (stages == 0)
        {
            if (HAS_ANY_FLAG(m_queue, COMPUTE_QUEUES))
            {
                stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }
            else
            {
                stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
        }
        return AddGenericBufferInput(name, stages, VK_ACCESS_SHADER_READ_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }

    RenderTextureResource& RenderPass::AddTextureInput(const String& name, VkPipelineStageFlags stages)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.ReadInPass(m_index);
        res.AddImageUsage(VK_IMAGE_USAGE_SAMPLED_BIT);

        // support duplicate inputs
        auto itr = eastl::find_if(begin(m_genericTextures), end(m_genericTextures), [&](const AccessedTextureResource& acc)
            {
                return acc.texture == &res;
            });

        // return the input if it already exists
        if (itr != end(m_genericTextures))
        {
            return *itr->texture;
        }

        // add a new input
        AccessedTextureResource acc;
        acc.texture = &res;
        acc.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        acc.access = VK_ACCESS_SHADER_READ_BIT;

        if (stages != 0)
        {
            acc.stages = stages;
        }
        else if (HAS_ANY_FLAG(m_queue, COMPUTE_QUEUES))
        {
            acc.stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
        else
        {
            acc.stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        m_genericTextures.push_back(acc);
        return res;
    }

    RenderTextureResource& RenderPass::AddBlitTextureReadOnlyInput(const String& name)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.ReadInPass(m_index);
        res.AddImageUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

        AccessedTextureResource acc;
        acc.texture = &res;
        acc.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        acc.access = VK_ACCESS_TRANSFER_READ_BIT;
        acc.stages = VK_PIPELINE_STAGE_TRANSFER_BIT;

        m_genericTextures.push_back(acc);
        return res;
    }

    RenderTextureResource& RenderPass::SetDepthStencilOutput(const String& name, const AttachmentInfo& info)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.WrittenInPass(m_index);
        res.SetAttachmentInfo(info);
        res.AddImageUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        m_depthStencilOutput = &res;
        return res;
    }

    RenderTextureResource& RenderPass::AddColorOutput(const String& name, const AttachmentInfo& info, const String& input)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.WrittenInPass(m_index);
        res.SetAttachmentInfo(info);
        res.AddImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

        if (info.levels != 1)
        {
            res.AddImageUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        }

        m_colorOutputs.push_back(&res);

        if (!input.empty())
        {
            auto& inputRes = m_graph.GetTextureResource(input);
            inputRes.ReadInPass(m_index);
            inputRes.AddImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
            m_colorInputs.push_back(&inputRes);
            m_colorScaleInputs.push_back(nullptr);
        }
        else
        {
            m_colorInputs.push_back(nullptr);
            m_colorScaleInputs.push_back(nullptr);
        }

        return res;
    }

    RenderTextureResource& RenderPass::AddResolveOutput(const String& name, const AttachmentInfo& info)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.WrittenInPass(m_index);
        res.SetAttachmentInfo(info);
        res.AddImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        m_resolveOutputs.push_back(&res);
        return res;
    }

    RenderBufferResource& RenderPass::AddStorageOutput(const String& name, const BufferInfo& info, const String& input)
    {
        auto& res = m_graph.GetBufferResource(name);
        res.AddQueue(m_queue);
        res.SetBufferInfo(info);
        res.WrittenInPass(m_index);
        res.AddBufferUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        m_storageOutputs.push_back(&res);

        if (!input.empty())
        {
            auto& inputRes = m_graph.GetBufferResource(input);
            inputRes.ReadInPass(m_index);
            inputRes.AddBufferUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
            m_storageInputs.push_back(&inputRes);
        }
        else
        {
            m_storageInputs.push_back(nullptr);
        }

        return res;
    }

    RenderTextureResource& RenderPass::AddStorageTextureOutput(const String& name, const AttachmentInfo& info, const String& input)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.WrittenInPass(m_index);
        res.SetAttachmentInfo(info);
        res.AddImageUsage(VK_IMAGE_USAGE_STORAGE_BIT);
        m_storageTextureOutputs.push_back(&res);

        if (!input.empty())
        {
            auto& inputRes = m_graph.GetTextureResource(input);
            inputRes.ReadInPass(m_index);
            inputRes.AddImageUsage(VK_IMAGE_USAGE_STORAGE_BIT);
            m_storageTextureInputs.push_back(&inputRes);
        }
        else
        {
            m_storageTextureInputs.push_back(nullptr);
        }

        return res;
    }

    RenderTextureResource& RenderPass::AddBlitTextureOutput(const String& name, const AttachmentInfo& info, const String& input)
    {
        auto& res = m_graph.GetTextureResource(name);
        res.AddQueue(m_queue);
        res.WrittenInPass(m_index);
        res.SetAttachmentInfo(info);
        res.AddImageUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        m_blitTextureOutputs.push_back(&res);

        if (!input.empty())
        {
            auto& inputRes = m_graph.GetTextureResource(input);
            inputRes.ReadInPass(m_index);
            inputRes.AddImageUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
            m_blitTextureInputs.push_back(&inputRes);
        }
        else
        {
            m_blitTextureInputs.push_back(nullptr);
        }

        return res;
    }

    void RenderPass::AddFakeResourceWriteAlias(const String& from, const String& to)
    {
        auto& fromRes = m_graph.GetTextureResource(from);
        auto& toRes = m_graph.GetTextureResource(to);
        toRes = fromRes;
        toRes.GetReadPasses().clear();
        toRes.GetWritePasses().clear();
        toRes.WrittenInPass(m_index);

        m_fakeResourceAliases.emplace_back(&fromRes, &toRes);
    }
}