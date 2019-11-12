#pragma once

#include "Mantis.h"

#include "RenderPass.h"
#include "RenderPassInfo.h"
#include "Renderer/RendererConfig.h"
#include "Renderer/Buffer/Buffer.h"
#include "Renderer/Image/Image.h"
#include "Renderer/Commands/CommandBuffer.h"

#include <assert.h>

namespace Mantis
{
    /// <summary>
    /// Manages renderpasses and their dependencies, automatically handling
    /// transitions between them where possible.
    /// </summary>
    class RenderGraph :
        public NonCopyable
    {
    public:
        /// <summary>
        /// Creates a new render graph.
        /// </summary>
        RenderGraph();

        /// <summary>
        /// Adds a renderpass to the graph.
        /// </summary>
        /// <param name="name">The name of the renderpass.</param>
        /// <param name="queue">The queue to execute the renderpass on.</param>
        RenderPass& AddPass(const String& name, RenderGraphQueue queue);

        /// <summary>
        /// Sets the final output source.
        /// </summary>
        /// <param name="name">The name of the output attachment.</param>
        void SetBackbufferSource(const String& name);

        /// <summary>
        /// Gets the dimensions of the graph output.
        /// </summary>
        const ResourceDimensions& GetBackbufferDimensions() const
        {
            return m_swapchainDimensions;
        }

        /// <summary>
        /// Sets the dimensions of the graph output.
        /// </summary>
        /// <param name="dim">The new output dimensions.</param>
        void SetBackbufferDimensions(const ResourceDimensions& dim)
        {
            m_swapchainDimensions = dim;
        }

        /// <summary>
        /// Resets the graph.
        /// </summary>
        void Reset();

        /// <summary>
        /// Bakes the graph from the current passes.
        /// </summary>
        void Bake();

        /// <summary>
        /// Prints the graph details to the log file.
        /// </summary>
        void Log();

        void SetupAttachments(Vulkan::ImageView* swapchain);
        void EnqueueRenderPasses();

        RenderTextureResource& GetTextureResource(const String& name);
        RenderBufferResource& GetBufferResource(const String& name);

        Vulkan::ImageView& GetPhysicalTextureResource(uint32_t index)
        {
            assert(index != RenderResource::Unused);
            assert(m_physicalAttachments[index]);
            return *m_physicalAttachments[index];
        }

        Vulkan::ImageView* GetPhysicalHistoryTextureResource(uint32_t index)
        {
            assert(index != RenderResource::Unused);
            if (!m_physicalHistoryImageAttachments[index])
            {
                return nullptr;
            }
            return &m_physicalHistoryImageAttachments[index]->get_view();
        }

        Buffer& GetPhysicalBufferResource(uint32_t index)
        {
            assert(index != RenderResource::Unused);
            assert(m_physicalBuffers[index]);
            return *m_physicalBuffers[index];
        }

        Vulkan::ImageView& GetPhysicalTextureResource(const RenderTextureResource& resource)
        {
            assert(resource.GetPhysicalIndex() != RenderResource::Unused);
            return GetPhysicalTextureResource(resource.GetPhysicalIndex());
        }

        Vulkan::ImageView* MaybeGetPhysicalTextureResource(RenderTextureResource* resource)
        {
            if (resource && resource->GetPhysicalIndex() != RenderResource::Unused)
            {
                return &GetPhysicalTextureResource(*resource);
            }
            return nullptr;
        }

        Vulkan::ImageView* GetPhysicalHistoryTextureResource(const RenderTextureResource& resource)
        {
            return GetPhysicalHistoryTextureResource(resource.GetPhysicalIndex());
        }

        Buffer& GetPhysicalBufferResource(const RenderBufferResource& resource)
        {
            assert(resource.GetPhysicalIndex() != RenderResource::Unused);
            return GetPhysicalBufferResource(resource.GetPhysicalIndex());
        }

        Buffer* MaybeGetPhysicalBufferResource(RenderBufferResource* resource)
        {
            if (resource && resource->GetPhysicalIndex() != RenderResource::Unused)
            {
                return &GetPhysicalBufferResource(*resource);
            }
            return nullptr;
        }

        // for keeping feed-back resources alive during rebaking
        eastl::shared_ptr<Buffer> ConsumePersistentPhysicalBufferResource(uint32_t index) const;
        void InstallPersistentPhysicalBufferResource(uint32_t index, eastl::shared_ptr<Buffer> buffer);

        // utility to consume all physical buffer handles and install them
        eastl::vector<eastl::shared_ptr<Buffer>> ConsumePhysicalBuffers() const;
        void InstallPhysicalBuffers(eastl::vector<eastl::shared_ptr<Buffer>> buffers);

        /// <summary>
        /// The default queue for post effects.
        /// </summary>
        static MANTIS_INLINE RenderGraphQueue GetDefaultPostGraphicsQueue()
        {
            if (RendererConfig::Get().useAsyncComputePost && !RendererConfig::Get().renderGraphForceSingleQueue)
            {
                return RenderGraphQueue::AsyncGraphics;
            }
            else
            {
                return RenderGraphQueue::Graphics;
            }
        }

        /// <summary>
        /// The default queue for compute.
        /// </summary>
        static MANTIS_INLINE RenderGraphQueue GetDefaultComputeQueue()
        {
            if (RendererConfig::Get().renderGraphForceSingleQueue)
            {
                return RenderGraphQueue::Compute;
            }
            else
            {
                return RenderGraphQueue::AsyncCompute;
            }
        }

    private:
        struct Barrier
        {
            uint32_t resourceIndex;
            VkImageLayout layout;
            VkAccessFlags access;
            VkPipelineStageFlags stages;
            bool history;
        };

        struct Barriers
        {
            eastl::vector<Barrier> invalidate;
            eastl::vector<Barrier> flush;
        };

        struct ColorClearRequest
        {
            RenderPass* pass;
            VkClearColorValue* target;
            uint32_t index;
        };

        struct DepthClearRequest
        {
            RenderPass* pass;
            VkClearDepthStencilValue* target;
        };

        struct ScaledClearRequests
        {
            uint32_t target;
            uint32_t physicalResource;
        };

        struct MipmapRequests
        {
            uint32_t physicalResource;
            VkPipelineStageFlags stages;
            VkAccessFlags access;
            VkImageLayout layout;
        };

        struct PhysicalPass
        {
            eastl::vector<uint32_t> passes;
            eastl::vector<uint32_t> discards;
            eastl::vector<Barrier> invalidate;
            eastl::vector<Barrier> flush;
            eastl::vector<Barrier> history;
            eastl::vector<eastl::pair<uint32_t, uint32_t>> aliasTransfer;

            RenderPassInfo renderPassInfo;
            eastl::vector<RenderPassInfo::Subpass> subpasses;
            eastl::vector<uint32_t> physicalColorAttachments;
            uint32_t physicalDepthStencilAttachment = RenderResource::Unused;

            eastl::vector<ColorClearRequest> colorClearRequests;
            DepthClearRequest depthClearRequest;

            eastl::vector<eastl::vector<ScaledClearRequests>> scaledClearRequests;
            eastl::vector<MipmapRequests> mipmapRequests;
            uint32_t layers = 1;
        };

        struct PipelineEvent
        {
            Vulkan::PipelineEvent event;
            // need two separate semaphores so we can wait in both queues independently
            Vulkan::Semaphore waitGraphicsSemaphore;
            Vulkan::Semaphore waitComputeSemaphore;

            // stages to wait for are stored inside the events
            VkAccessFlags toFlushAccess = 0;
            VkAccessFlags invalidatedInStage[32] = {};
            VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        };

        void OnSwapchainChanged(const Vulkan::SwapchainParameterEvent& e);
        void OnSwapchainDestroyed(const Vulkan::SwapchainParameterEvent& e);

        ResourceDimensions GetResourceDimensions(const RenderBufferResource& resource) const;
        ResourceDimensions GetResourceDimensions(const RenderTextureResource& resource) const;

        void SetupPhysicalBuffer(uint32_t attachment);
        void SetupPhysicalImage(uint32_t attachment);

        void EnqueueScaledRequests(CommandBuffer& cmd, const eastl::vector<ScaledClearRequests>& requests);
        void EnqueueMipmapRequests(CommandBuffer& cmd, const eastl::vector<MipmapRequests>& requests);

        void ValidatePasses();
        void TraverseDependencies(const RenderPass& pass, uint32_t stackCount);
        void FilterPasses(eastl::vector<uint32_t>& list);
        void ReorderPasses(eastl::vector<uint32_t>& flattenedPasses);
        void BuildPhysicalResources();
        void BuildPhysicalPasses();
        void BuildTransients();
        void BuildRenderPassInfo();
        void BuildBarriers();
        void BuildPhysicalBarriers();
        void BuildAliases();

        void DependPassesRecursive(
            const RenderPass& pass,
            const eastl::unordered_set<uint32_t>& writtenPasses,
            uint32_t stackCount,
            bool noCheck,
            bool ignoreSelf,
            bool mergeDependency
        );

        bool DependsOnPass(uint32_t dstPass, uint32_t srcPass);

        static bool NeedInvalidate(const Barrier& barrier, const PipelineEvent& event);

        ResourceDimensions m_swapchainDimensions;
        Vulkan::ImageView* m_swapchainAttachment = nullptr;
        uint32_t m_swapchainPhysicalIndex = RenderResource::Unused;

        eastl::vector<eastl::unique_ptr<RenderPass>> m_passes;
        eastl::vector<eastl::unique_ptr<RenderResource>> m_resources;
        eastl::unordered_map<String, uint32_t> m_passToIndex;
        eastl::unordered_map<String, uint32_t> m_resourceToIndex;
        String m_backbufferSource;

        eastl::vector<ResourceDimensions> m_physicalDimensions;
        eastl::vector<Vulkan::ImageView*> m_physicalAttachments;
        eastl::vector<eastl::shared_ptr<Buffer>> m_physicalBuffers;
        eastl::vector<eastl::shared_ptr<Image>> m_physicalImageAttachments;
        eastl::vector<eastl::shared_ptr<Image>> m_physicalHistoryImageAttachments;
        eastl::vector<PipelineEvent> m_physicalEvents;
        eastl::vector<PipelineEvent> m_physicalHistoryEvents;

        eastl::vector<uint32_t> m_passStack;
        eastl::vector<eastl::unordered_set<uint32_t>> m_passDependencies;
        eastl::vector<eastl::unordered_set<uint32_t>> m_passMergeDependencies;

        eastl::vector<PhysicalPass> m_physicalPasses;
        eastl::vector<bool> m_physicalImageHasHistory;
        eastl::vector<Barriers> m_passBarriers;
        eastl::vector<uint32_t> m_physicalAliases;
    };
}
