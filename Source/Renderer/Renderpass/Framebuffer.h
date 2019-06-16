#pragma once

#include "Mantis.h"
#include "Swapchain.h"

namespace Mantis
{
    class ImageDepth;
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
            const VkSampleCountFlagBits& samples = VK_SAMPLE_COUNT_1_BIT
        );

        ~Framebuffers();

        Image2d* GetAttachment(const uint32_t& index) const { return m_imageAttachments[index].get(); }

        const eastl::vector<VkFramebuffer>& GetFramebuffers() const { return m_framebuffers; }

    private:
        eastl::vector<eastl::unique_ptr<Image2d>> m_imageAttachments;
        eastl::vector<VkFramebuffer> m_framebuffers;
    };
}
