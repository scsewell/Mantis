#include "stdafx.h"
#include "Framebuffer.h"

#include "Renderer/Renderer.h"
#include "Renderer/RenderStage.h"
#include "Renderpass.h"

#define LOG_TAG MANTIS_TEXT("Framebuffer")

namespace Mantis
{
    Framebuffers::Framebuffers(
        const Vector2Int& extent,
        const RenderStage& renderStage,
        const Renderpass& renderPass,
        const Swapchain& swapchain,
        const ImageDepth& depthStencil,
        const VkSampleCountFlagBits& samples
    ) : 
        m_imageAttachments({}),
        m_framebuffers({})
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        for (const auto& attachment : renderStage.GetAttachments())
        {
            auto attachmentSamples = attachment.IsMultisampled() ? samples : VK_SAMPLE_COUNT_1_BIT;

            switch (attachment.GetType())
            {
                case Attachment::Type::Image:
                {
                    ImageFramebuffer* image = ImageFramebuffer::Create2D({extent.x, extent.y}, attachment.GetFormat(), attachmentSamples, 1);
                    m_imageAttachments.emplace_back(eastl::unique_ptr<ImageFramebuffer>(image));
                    break;
                }
                case Attachment::Type::Depth:
                    m_imageAttachments.emplace_back(nullptr);
                    break;
                case Attachment::Type::Swapchain:
                    m_imageAttachments.emplace_back(nullptr);
                    break;
            }
        }

        m_framebuffers.resize(swapchain.GetImageCount());

        for (uint32_t i = 0; i < swapchain.GetImageCount(); i++)
        {
            eastl::vector<VkImageView> attachments;

            for (const auto& attachment : renderStage.GetAttachments())
            {
                switch (attachment.GetType())
                {
                    case Attachment::Type::Image:
                        attachments.emplace_back(GetAttachment(attachment.GetBinding())->GetView());
                        break;
                    case Attachment::Type::Depth:
                        attachments.emplace_back(depthStencil.GetView());
                        break;
                    case Attachment::Type::Swapchain:
                        attachments.emplace_back(swapchain.GetImageViews().at(i));
                        break;
                }
            }

            VkFramebufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            createInfo.renderPass = renderPass.GetRenderpass();
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = static_cast<uint32_t>(extent.x);
            createInfo.height = static_cast<uint32_t>(extent.y);
            createInfo.layers = 1;

            if (Renderer::Check(vkCreateFramebuffer(*logicalDevice, &createInfo, nullptr, &m_framebuffers[i])))
            {
                Logger::ErrorT(LOG_TAG, "Failed to crete framebuffer!");
            }
        }
    }

    Framebuffers::~Framebuffers()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        for (const auto& framebuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(*logicalDevice, framebuffer, nullptr);
        }
    }
}
