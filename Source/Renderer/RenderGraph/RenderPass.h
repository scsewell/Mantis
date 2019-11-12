#pragma once

#include "Mantis.h"

#include "Renderer/RendererConfig.h"
#include "Renderer/Commands/CommandBuffer.h"

namespace Mantis
{
    class RenderGraph;

    /// <summary>
    /// How to interperet the size of an attachment.
    /// </summary>
    enum SizeMode
    {
        /// <summary>
        /// The size is the resolution in pixels.
        /// </summary>
        Absolute,
        /// <summary>
        /// The size is a ratio of the swapchain resolution.
        /// </summary>
        SwapchainRelative,
        /// <summary>
        /// The size is a ratio of a named input's resolution.
        /// </summary>
        InputRelative,
    };

    /// <summary>
    /// The queue used to execute a renderpass.
    /// </summary>
    enum RenderGraphQueue : int
    {
        Graphics        = 1 << 0,
        Compute         = 1 << 1,
        AsyncGraphics   = 1 << 2,
        AsyncCompute    = 1 << 3,
    };
    ENUM_IS_FLAGS(RenderGraphQueue);

    /// <summary>
    /// Describes an attachment.
    /// </summary>
    struct AttachmentInfo
    {
        /// <summary>
        /// How to interperet the size of an attachment.
        /// </summary>
        SizeMode sizeMode = SizeMode::SwapchainRelative;
        /// <summary>
        /// The name of the input the size is relative to when sizeMode is InputRelative.
        /// </summary>
        String sizeRelativeName;
        float sizeX = 1.0f;
        float sizeY = 1.0f;
        float sizeZ = 0.0f;
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint32_t samples = 1;
        uint32_t levels = 1;
        uint32_t layers = 1;
        VkImageUsageFlags auxUsage = 0;
        bool persistent = true;
        /// <summary>
        /// Allow interpreting the attachment as both linear or sRGB.
        /// </summary>
        bool aliasUnormSrgb = false;
    };

    /// <summary>
    /// Describes a buffer.
    /// </summary>
    struct BufferInfo
    {
        VkDeviceSize size = 0;
        VkBufferUsageFlags usage = 0;
        bool persistent = true;

        bool operator == (const BufferInfo& other) const
        {
            return size == other.size &&
                usage == other.usage &&
                persistent == other.persistent;
        }

        bool operator != (const BufferInfo& other) const
        {
            return !(*this == other);
        }
    };

    struct ResourceDimensions
    {
        String name;

        VkFormat format = VK_FORMAT_UNDEFINED;
        BufferInfo bufferInfo;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t samples = 1;
        uint32_t layers = 1;
        uint32_t levels = 1;
        bool transient = false;
        bool unormSrgb = false;
        bool persistent = true;
        RenderGraphQueue queues = static_cast<RenderGraphQueue>(0);
        VkImageUsageFlags imageUsage = 0;

        bool operator == (const ResourceDimensions& other) const
        {
            // imageUsage and queues are deliberately not part of this test
            return format == other.format &&
                width == other.width &&
                height == other.height &&
                depth == other.depth &&
                layers == other.layers &&
                levels == other.levels &&
                bufferInfo == other.bufferInfo &&
                transient == other.transient &&
                persistent == other.persistent &&
                unormSrgb == other.unormSrgb;
        }

        bool operator != (const ResourceDimensions& other) const
        {
            return !(*this == other);
        }

        bool UsesSemaphore() const
        {
            // if more than one queue is used for a resource, we need to use semaphores
            auto physicalQueues = queues;

            // regular compute uses regular graphics queue
            if (physicalQueues & RenderGraphQueue::Compute)
            {
                physicalQueues &= ~RenderGraphQueue::Compute;
                physicalQueues |= RenderGraphQueue::Graphics;
            }
            return (physicalQueues & (physicalQueues - 1)) != 0;
        }

        bool IsStorageImage() const
        {
            return (imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
        }

        bool IsBufferLike() const
        {
            return IsStorageImage() || (bufferInfo.size != 0);
        }
    };

    class RenderResource
    {
    public:
        enum struct Type
        {
            Buffer,
            Texture,
        };

        enum { Unused = ~0u };

        RenderResource(const String& name, Type type, uint32_t index)
            : m_name(name)
            , m_type(type)
            , m_index(index)
            , m_physicalIndex(Unused)
        {
        }

        virtual ~RenderResource() = default;

        const String& GetName() const
        {
            return m_name;
        }

        Type GetType() const
        {
            return m_type;
        }

        uint32_t GetIndex() const
        {
            return m_index;
        }

        uint32_t GetPhysicalIndex() const
        {
            return m_physicalIndex;
        }

        void SetPhysicalIndex(uint32_t index)
        {
            m_physicalIndex = index;
        }

        void WrittenInPass(uint32_t index)
        {
            m_writtenInPasses.insert(index);
        }

        void ReadInPass(uint32_t index)
        {
            m_readInPasses.insert(index);
        }

        const eastl::unordered_set<uint32_t>& GetReadPasses() const
        {
            return m_readInPasses;
        }

        const eastl::unordered_set<uint32_t>& GetWritePasses() const
        {
            return m_writtenInPasses;
        }

        eastl::unordered_set<uint32_t>& GetReadPasses()
        {
            return m_readInPasses;
        }

        eastl::unordered_set<uint32_t>& GetWritePasses()
        {
            return m_writtenInPasses;
        }

        RenderGraphQueue GetUsedQueues() const
        {
            return m_usedQueues;
        }

        void AddQueue(RenderGraphQueue queue)
        {
            m_usedQueues |= queue;
        }

    private:
        String m_name;
        Type m_type;
        uint32_t m_index;
        uint32_t m_physicalIndex;
        eastl::unordered_set<uint32_t> m_writtenInPasses;
        eastl::unordered_set<uint32_t> m_readInPasses;
        RenderGraphQueue m_usedQueues = static_cast<RenderGraphQueue>(0);
    };

    class RenderBufferResource : public RenderResource
    {
    public:
        explicit RenderBufferResource(const String& name, uint32_t index)
            : RenderResource(name, RenderResource::Type::Buffer, index)
        {
        }

        const BufferInfo& GetBufferInfo() const
        {
            return m_info;
        }

        void SetBufferInfo(const BufferInfo& info)
        {
            m_info = info;
        }

        VkBufferUsageFlags GetBufferUsage() const
        {
            return m_bufferUsage;
        }

        void AddBufferUsage(VkBufferUsageFlags flags)
        {
            m_bufferUsage |= flags;
        }

    private:
        BufferInfo m_info;
        VkBufferUsageFlags m_bufferUsage = 0;
    };

    class RenderTextureResource : public RenderResource
    {
    public:
        explicit RenderTextureResource(const String& name, uint32_t index)
            : RenderResource(name, RenderResource::Type::Texture, index)
        {
        }

        const AttachmentInfo& GetAttachmentInfo() const
        {
            return m_info;
        }

        AttachmentInfo& GetAttachmentInfo()
        {
            return m_info;
        }

        void SetAttachmentInfo(const AttachmentInfo& info)
        {
            m_info = info;
        }

        VkImageUsageFlags GetImageUsage() const
        {
            return m_imageUsage;
        }

        void AddImageUsage(VkImageUsageFlags flags)
        {
            m_imageUsage |= flags;
        }

        bool GetTransientState() const
        {
            return transient;
        }

        void SetTransientState(bool enable)
        {
            transient = enable;
        }

    private:
        AttachmentInfo m_info;
        VkImageUsageFlags m_imageUsage = 0;
        bool transient = false;
    };

    /// <summary>
    /// Represents a render pass in the render graph.
    /// </summary>
    class RenderPass
    {
    public:
        enum { Unused = ~0u };

        struct AccessedResource
        {
            VkPipelineStageFlags stages = 0;
            VkAccessFlags access = 0;
            VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        };

        struct AccessedTextureResource : AccessedResource
        {
            RenderTextureResource* texture = nullptr;
        };

        struct AccessedBufferResource : AccessedResource
        {
            RenderBufferResource* buffer = nullptr;
        };

        /// <summary>
        /// Creates a new render pass.
        /// </summary>
        /// <param name="graph">The graph this renderpass belongs to.</param>
        /// <param name="name">The name of the renderpass.</param>
        /// <param name="index">The index of this renderpass in the graph.</param>
        /// <param name="queue">The queue to execute this renderpass on.</param>
        RenderPass(RenderGraph& graph, const String name, uint32_t index, RenderGraphQueue queue)
            : m_graph(graph)
            , m_name(name)
            , m_index(index)
            , m_physicalPass(Unused)
            , m_queue(queue)
        {
        }

        /// <summary>
        /// Gets the name of this renderpass.
        /// </summary>
        const String& GetName() const
        {
            return m_name;
        }

        /// <summary>
        /// Sets the name of the renderpass.
        /// </summary>
        void SetName(const String& name)
        {
            m_name = name;
        }

        /// <summary>
        /// Gets the graph this renderpass belongs to.
        /// </summary>
        RenderGraph& GetGraph()
        {
            return m_graph;
        }

        /// <summary>
        /// Gets the queue to execute this renderpass on.
        /// </summary>
        RenderGraphQueue GetQueue() const
        {
            return m_queue;
        }

        /// <summary>
        /// Gets the index of this renderpass in the graph.
        /// </summary>
        uint32_t GetIndex() const
        {
            return m_index;
        }

        /// <summary>
        /// Gets the index of the physical renderpass in the graph.
        /// </summary>
        uint32_t GetPhysicalPassIndex() const
        {
            return m_physicalPass;
        }

        /// <summary>
        /// Sets the index of the physical renderpass in the graph.
        /// </summary>
        void SetPhysicalPassIndex(uint32_t index)
        {
            m_physicalPass = index;
        }

        /// <summary>
        /// Adds a depth stencil resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderTextureResource& SetDepthStencilInput(const String& name);
        /// <summary>
        /// Adds an attachment resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderTextureResource& AddAttachmentInput(const String& name);
        /// <summary>
        /// Adds a history texture resource. History inputs are not used in any particular pass, but next frame.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderTextureResource& AddHistoryInput(const String& name);
        /// <summary>
        /// Adds a vertex buffer resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderBufferResource& AddVertexBufferInput(const String& name);
        /// <summary>
        /// Adds an index buffer resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderBufferResource& AddIndexBufferInput(const String& name);
        /// <summary>
        /// Adds an indirect buffer resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderBufferResource& AddIndirectBufferInput(const String& name);
        /// <summary>
        /// Adds a uniform buffer resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="stages">The stages at which the resource is to be used.</param>
        RenderBufferResource& AddUniformInput(const String& name, VkPipelineStageFlags stages = 0);
        /// <summary>
        /// Adds a read-only storage buffer resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="stages">The stages at which the resource is to be used.</param>
        RenderBufferResource& AddStorageReadOnlyInput(const String& name, VkPipelineStageFlags stages = 0);
        /// <summary>
        /// Adds a texture resource as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="stages">The stages at which the resource is to be used.</param>
        RenderTextureResource& AddTextureInput(const String& name, VkPipelineStageFlags stages = 0);
        /// <summary>
        /// Adds a read-only texture resource to blit as input to this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        RenderTextureResource& AddBlitTextureReadOnlyInput(const String& name);

        /// <summary>
        /// Adds a depth stencil resource as output from this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="info">The description of the attachment.</param>
        RenderTextureResource& SetDepthStencilOutput(const String& name, const AttachmentInfo& info);
        /// <summary>
        /// Adds a color attachment resource as output from this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="info">The description of the attachment.</param>
        /// <param name="input">The corresponding input resource, if applicable.</param>
        RenderTextureResource& AddColorOutput(const String& name, const AttachmentInfo& info, const String& input = "");
        /// <summary>
        /// Adds a resolve attachment resource as output from this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="info">The description of the attachment.</param>
        RenderTextureResource& AddResolveOutput(const String& name, const AttachmentInfo& info);
        /// <summary>
        /// Adds a storage buffer resource as output from this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="info">The description of the attachment.</param>
        /// <param name="input">The corresponding input resource, if applicable.</param>
        RenderBufferResource& AddStorageOutput(const String& name, const BufferInfo& info, const String& input = "");
        /// <summary>
        /// Adds a storage texture attachment resource as output from this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="info">The description of the attachment.</param>
        /// <param name="input">The corresponding input resource, if applicable.</param>
        RenderTextureResource& AddStorageTextureOutput(const String& name, const AttachmentInfo& info, const String& input = "");
        /// <summary>
        /// Adds a texture resource to blit to as output from this pass.
        /// </summary>
        /// <param name="name">The name of the resource.</param>
        /// <param name="info">The description of the attachment.</param>
        /// <param name="input">The corresponding input resource, if applicable.</param>
        RenderTextureResource& AddBlitTextureOutput(const String& name, const AttachmentInfo& info, const String& input = "");

        void AddFakeResourceWriteAlias(const String& from, const String& to);

        /// <summary>
        /// Makes a color input use scaling to account for a resolution difference.
        /// </summary>
        /// <param name="index">The index of the color input to scale.</param>
        void MakeColorInputScaled(uint32_t index)
        {
            eastl::swap(m_colorScaleInputs[index], m_colorInputs[index]);
        }

        const eastl::vector<RenderTextureResource*>& GetColorInputs() const
        {
            return m_colorInputs;
        }

        const eastl::vector<RenderTextureResource*>& GetColorScaleInputs() const
        {
            return m_colorScaleInputs;
        }

        const eastl::vector<RenderTextureResource*>& GetColorOutputs() const
        {
            return m_colorOutputs;
        }

        const eastl::vector<RenderTextureResource*>& GetResolveOutputs() const
        {
            return m_resolveOutputs;
        }

        const eastl::vector<RenderTextureResource*>& GetStorageTextureInputs() const
        {
            return m_storageTextureInputs;
        }

        const eastl::vector<RenderTextureResource*>& GetStorageTextureOutputs() const
        {
            return m_storageTextureOutputs;
        }

        const eastl::vector<RenderTextureResource*>& GetBlitTextureInputs() const
        {
            return m_blitTextureInputs;
        }

        const eastl::vector<RenderTextureResource*>& GetBlitTextureOutputs() const
        {
            return m_blitTextureOutputs;
        }

        const eastl::vector<RenderTextureResource*>& GetAttachmentInputs() const
        {
            return m_attachmentInputs;
        }

        const eastl::vector<RenderTextureResource*>& GetHistoryInputs() const
        {
            return m_historyInputs;
        }

        const eastl::vector<RenderBufferResource*>& GetStorageInputs() const
        {
            return m_storageInputs;
        }

        const eastl::vector<RenderBufferResource*>& GetStorageOutputs() const
        {
            return m_storageOutputs;
        }

        const eastl::vector<AccessedTextureResource>& GetGenericTextureInputs() const
        {
            return m_genericTextures;
        }

        const eastl::vector<AccessedBufferResource>& GetGenericBufferInputs() const
        {
            return m_genericBuffers;
        }

        const eastl::vector<eastl::pair<RenderTextureResource*, RenderTextureResource*>>& GetFakeResourceAliases() const
        {
            return m_fakeResourceAliases;
        }

        RenderTextureResource* GetDepthStencilInput() const
        {
            return m_depthStencilInput;
        }

        RenderTextureResource* GetDepthStencilOutput() const
        {
            return m_depthStencilOutput;
        }

        bool NeedRenderPass() const
        {
            if (m_needRenderPassCB)
            {
                return m_needRenderPassCB();
            }
            return true;
        }

        bool MayNotNeedRenderPass() const
        {
            return bool(m_needRenderPassCB);
        }

        bool GetClearColor(uint32_t index, VkClearColorValue* value = nullptr)
        {
            if (m_GetClearColorCB)
            {
                return m_GetClearColorCB(index, value);
            }
            return false;
        }

        bool GetClearDepthStencil(VkClearDepthStencilValue* value = nullptr)
        {
            if (m_GetClearDepthStencilCB)
            {
                return m_GetClearDepthStencilCB(value);
            }
            return false;
        }

        void BuildRenderPass(CommandBuffer& cmd, uint32_t layer)
        {
            if (m_buildRenderPassLayeredCB)
            {
                m_buildRenderPassLayeredCB(layer, cmd);
            }
            else if (m_buildRenderPassCB)
            {
                m_buildRenderPassCB(cmd);
            }
        }

        void SetNeedRenderPass(eastl::function<bool()> func)
        {
            m_needRenderPassCB = eastl::move(func);
        }

        void SetBuildRenderPass(eastl::function<void(CommandBuffer&)> func)
        {
            m_buildRenderPassCB = eastl::move(func);
        }

        void SetBuildRenderPassLayered(eastl::function<void(uint32_t, CommandBuffer&)> func)
        {
            m_buildRenderPassLayeredCB = eastl::move(func);
        }

        void SetGetClearColor(eastl::function<bool(uint32_t, VkClearColorValue*)> func)
        {
            m_GetClearColorCB = eastl::move(func);
        }

        void SetGetClearDepthStencil(eastl::function<bool(VkClearDepthStencilValue*)> func)
        {
            m_GetClearDepthStencilCB = eastl::move(func);
        }

    private:
        RenderBufferResource& AddGenericBufferInput(
            const String& name,
            VkPipelineStageFlags stages,
            VkAccessFlags access,
            VkBufferUsageFlags usage
        );

        String m_name;
        RenderGraph& m_graph;
        uint32_t m_index;
        uint32_t m_physicalPass;
        RenderGraphQueue m_queue;

        eastl::function<void(CommandBuffer&)> m_buildRenderPassCB;
        eastl::function<void(uint32_t, CommandBuffer&)> m_buildRenderPassLayeredCB;
        eastl::function<bool()> m_needRenderPassCB;
        eastl::function<bool(VkClearDepthStencilValue*)> m_GetClearDepthStencilCB;
        eastl::function<bool(uint32_t, VkClearColorValue*)> m_GetClearColorCB;

        eastl::vector<RenderTextureResource*> m_colorScaleInputs;
        eastl::vector<RenderTextureResource*> m_colorInputs;
        eastl::vector<RenderTextureResource*> m_colorOutputs;
        eastl::vector<RenderTextureResource*> m_resolveOutputs;
        eastl::vector<RenderTextureResource*> m_storageTextureInputs;
        eastl::vector<RenderTextureResource*> m_storageTextureOutputs;
        eastl::vector<RenderTextureResource*> m_blitTextureInputs;
        eastl::vector<RenderTextureResource*> m_blitTextureOutputs;
        eastl::vector<RenderTextureResource*> m_attachmentInputs;
        eastl::vector<RenderTextureResource*> m_historyInputs;
        eastl::vector<RenderBufferResource*> m_storageOutputs;
        eastl::vector<RenderBufferResource*> m_storageInputs;
        eastl::vector<AccessedTextureResource> m_genericTextures;
        eastl::vector<AccessedBufferResource> m_genericBuffers;
        RenderTextureResource* m_depthStencilInput = nullptr;
        RenderTextureResource* m_depthStencilOutput = nullptr;

        eastl::vector<eastl::pair<RenderTextureResource*, RenderTextureResource*>> m_fakeResourceAliases;
    };
}
