#include "Renderpass.h"

#include "Subpass.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderStage.h"

#define LOG_TAG MANTIS_TEXT("Renderpass")

namespace Mantis
{
    Renderpass::Renderpass(
        const RenderStage& renderStage,
        const VkFormat& depthFormat,
        const VkFormat& surfaceFormat,
        const VkSampleCountFlagBits& samples
    ) :
        m_renderpass(VK_NULL_HANDLE)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        // create the renderpasses attachment descriptions
        eastl::vector<VkAttachmentDescription> attachmentDescriptions;

        for (const auto& attachment : renderStage.GetAttachments())
        {
            auto attachmentSamples = attachment.IsMultisampled() ? samples : VK_SAMPLE_COUNT_1_BIT;

            VkAttachmentDescription attachmentDescription = {};
            attachmentDescription.samples = attachmentSamples;
            attachmentDescription.loadOp = static_cast<VkAttachmentLoadOp>(attachment.GetLoadOp());
            attachmentDescription.storeOp = static_cast<VkAttachmentStoreOp>(attachment.GetStoreOp());
            attachmentDescription.stencilLoadOp = static_cast<VkAttachmentLoadOp>(Attachment::LoadOp::DontCare);
            attachmentDescription.stencilStoreOp = static_cast<VkAttachmentStoreOp>(Attachment::StoreOp::DontCare);
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            switch (attachment.GetType())
            {
                case Attachment::Type::Image:
                    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    attachmentDescription.format = attachment.GetFormat();
                    break;
                case Attachment::Type::Depth:
                    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    attachmentDescription.format = depthFormat;
                    break;
                case Attachment::Type::Swapchain:
                    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    attachmentDescription.format = surfaceFormat;
                    break;
            }

            attachmentDescriptions.emplace_back(attachmentDescription);
        }

        // describe each subpass
        eastl::vector<eastl::unique_ptr<SubpassDescription>> subpasses;

        for (const auto& subpass : renderStage.GetSubpasses())
        {
            eastl::vector<VkAttachmentReference> inputAttachments;
            eastl::vector<VkAttachmentReference> colorAttachments;
            eastl::vector<VkAttachmentReference> resolveAttachments;
            eastl::optional<VkAttachmentReference> depthAttachment;
            eastl::vector<uint32_t> preserveAttachments;

            for (const auto& attachmentRef : subpass.GetAttachmentRefs())
            {
                auto attachment = renderStage.GetAttachment(attachmentRef.binding);

                if (!attachment)
                {
                    Logger::ErrorTF(LOG_TAG, "Failed to find a renderpass attachment bound to: %i!", attachmentRef.binding);
                    continue;
                }

                VkAttachmentReference attachmentReference = {};
                attachmentReference.attachment = attachmentRef.binding;

                switch (attachmentRef.mode)
                {
                    case AttachmentMode::Input:
                        attachmentReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        inputAttachments.push_back(attachmentReference);
                        break;
                    case AttachmentMode::Color:
                        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        colorAttachments.push_back(attachmentReference);
                        break;
                    case AttachmentMode::Resolve:
                        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        resolveAttachments.push_back(attachmentReference);
                        break;
                    case AttachmentMode::Depth:
                        attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        depthAttachment = attachmentReference;
                        break;
                    case AttachmentMode::Preserve:
                        preserveAttachments.push_back(attachmentRef.binding);
                        break;
                    default:
                        Logger::ErrorTF(LOG_TAG, "Unsupported attachment mode: %i!", attachmentRef.mode);
                        break;
                }
            }

            subpasses.push_back(eastl::make_unique<SubpassDescription>(
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                inputAttachments, 
                colorAttachments,
                resolveAttachments,
                depthAttachment,
                preserveAttachments
            ));
        }

        eastl::vector<VkSubpassDescription> subpassDescriptions(subpasses.size());
        for (const auto& subpass : subpasses)
        {
            subpassDescriptions.push_back(subpass->GetSubpassDescription());
        }

        // create the dependancies for the subpasses
        eastl::vector<VkSubpassDependency> subpassDependencies;

        for (const auto& subpass : renderStage.GetSubpasses())
        {
            auto dependencies = subpass.GetDependencies();

            if (dependencies.size() == 0)
            {
                // If the subpass has no dependencies on other subpasses, createa an external subpass dependency
                VkSubpassDependency dependency = {};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = subpass.GetBinding();
                dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                subpassDependencies.emplace_back(dependency);
            }
            else
            {
                // add dependencies on other subpasses
                for (const auto& dependencySubpass : dependencies)
                {
                    VkSubpassDependency dependency = {};
                    dependency.srcSubpass = dependencySubpass->GetBinding();
                    dependency.dstSubpass = subpass.GetBinding();
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                    subpassDependencies.emplace_back(dependency);
                }
            }
        }

        // There should be only one subpass that is not a dependency, which is dependant on all
        // other subpasses. This must also have the highest binding index. We can therefore assume
        // that the last subpass gets the final dependency to the external source.
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = renderStage.GetSubpasses().back().GetBinding();
        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDependencies.emplace_back(dependency);

        // create the render pass
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
        renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
        renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassCreateInfo.pDependencies = subpassDependencies.data();
        
        if (Renderer::Check(vkCreateRenderPass(*logicalDevice, &renderPassCreateInfo, nullptr, &m_renderpass)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create renderpass!");
        }
    }

    Renderpass::~Renderpass()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        vkDestroyRenderPass(*logicalDevice, m_renderpass, nullptr);
    }
}
