#pragma once

#include "Mantis.h"
#include "Swapchain.h"
#include "Renderer/Image/ImageFramebuffer.h"
#include "Renderer/Image/ImageDepth.h"

namespace Mantis
{
    class Renderpass;
    class RenderStage;
    
    class Framebuffers :
        public NonCopyable
    {
    public:
        Framebuffers(
            const Vector2Int& extent,
            const RenderStage& renderStage,
            const Renderpass& renderPass,
            const Swapchain& swapchain,
            const ImageDepth& depthStencil,
            const VkSampleCountFlagBits& samples
        );

        ~Framebuffers();

        /// <summary>
        /// Gets the image attachment for the given framebuffer index.
        /// </summary>
        /// <param name="index">The index in the framebuffers.</param>
        ImageFramebuffer* GetAttachment(const uint32_t& index) const { return m_imageAttachments[index].get(); }

        /// <summary>
        /// Gets all of the underlying framebuffer instances.
        /// </summary>
        const eastl::vector<VkFramebuffer>& GetFramebuffers() const { return m_framebuffers; }

    private:
        eastl::vector<eastl::unique_ptr<ImageFramebuffer>> m_imageAttachments;
        eastl::vector<VkFramebuffer> m_framebuffers;
    };
}
