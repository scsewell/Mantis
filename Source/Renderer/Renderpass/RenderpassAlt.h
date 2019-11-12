#pragma once

#include "Mantis.h"

namespace Mantis
{
    class RenderStage;

    /// <summary>
    /// A class that represents a renderpass.
    /// </summary>
    class Renderpass
    {
    public:
        /// <summary>
        /// A class that represents a subpass.
        /// </summary>
        class SubpassDescription :
            public NonCopyable
        {
        public:
            /// <summary>
            /// Creates a new subpass description.
            /// </summary>
            /// <param name="bindPoint">The bind point of the subpass.</param>
            /// <param name="inputAttachments">The input attachments for this subpass.</param>
            /// <param name="colorAttachments">The color attachments for this subpass.</param>
            /// <param name="resolveAttachments">The resolve attachments for this subpass.</param>
            /// <param name="depthAttachment">The depth attachment for this subpass, if needed.</param>
            /// <param name="preserveAttachments">The attachments to preserve for this subpass.</param>
            SubpassDescription(
                const VkPipelineBindPoint& bindPoint,
                eastl::vector<VkAttachmentReference> inputAttachments,
                eastl::vector<VkAttachmentReference> colorAttachments,
                eastl::vector<VkAttachmentReference> resolveAttachments,
                const eastl::optional<VkAttachmentReference>& depthAttachment,
                eastl::vector<uint32_t> preserveAttachments
            ) :
                m_subpassDescription({}),
                m_inputAttachments(eastl::move(inputAttachments)),
                m_colorAttachments(eastl::move(colorAttachments)),
                m_resolveAttachments(eastl::move(resolveAttachments)),
                m_depthStencilAttachment({}),
                m_preserveAttachments(eastl::move(preserveAttachments))
            {
                m_subpassDescription.pipelineBindPoint = bindPoint;

                m_subpassDescription.inputAttachmentCount = static_cast<uint32_t>(m_inputAttachments.size());
                m_subpassDescription.pInputAttachments = m_inputAttachments.data();
                m_subpassDescription.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachments.size());
                m_subpassDescription.pColorAttachments = m_colorAttachments.data();
                m_subpassDescription.pResolveAttachments = m_resolveAttachments.data();

                if (depthAttachment)
                {
                    m_depthStencilAttachment = *depthAttachment;
                    m_subpassDescription.pDepthStencilAttachment = &m_depthStencilAttachment;
                }

                m_subpassDescription.preserveAttachmentCount = static_cast<uint32_t>(m_preserveAttachments.size());
                m_subpassDescription.pPreserveAttachments = m_preserveAttachments.data();
            }

            /// <summary>
            /// Gets the underlying subpass description instance.
            /// </summary>
            const VkSubpassDescription& GetSubpassDescription() const { return m_subpassDescription; }

        private:
            VkSubpassDescription m_subpassDescription;
            eastl::vector<VkAttachmentReference> m_inputAttachments;
            eastl::vector<VkAttachmentReference> m_colorAttachments;
            eastl::vector<VkAttachmentReference> m_resolveAttachments;
            VkAttachmentReference m_depthStencilAttachment;
            eastl::vector<uint32_t> m_preserveAttachments;
        };

        /// <summary>
        /// Creates a new renderpass.
        /// </summary>
        /// <param name="renderStage">The render stage for this renderpass.</param>
        /// <param name="depthFormat">The format of the depth buffer, if applicable.</param>
        /// <param name="surfaceFormat">The format of the presenation surface, if applicable.</param>
        /// <param name="samples">The number of MSAA samples to use for the attachments.</param>
        Renderpass(
            const RenderStage& renderStage,
            const VkFormat& depthFormat,
            const VkFormat& surfaceFormat,
            const VkSampleCountFlagBits& samples
        );

        /// <summary>
        /// Destroys the renderpass.
        /// </summary>
        ~Renderpass();

        /// <summary>
        /// Gets the underlying renderpass instance.
        /// </summary>
        operator const VkRenderPass& () const { return m_renderpass; }

        /// <summary>
        /// Gets the underlying renderpass instance.
        /// </summary>
        const VkRenderPass& GetRenderpass() const { return m_renderpass; }

    private:
        VkRenderPass m_renderpass;
    };
}