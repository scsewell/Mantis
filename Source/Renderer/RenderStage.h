#pragma once

#include "Mantis.h"

#include "Renderpass/Attachment.h"
#include "Renderpass/Subpass.h"
#include "Renderpass/Framebuffer.h"
#include "Renderpass/Swapchain.h"
#include "Descriptor/Descriptor.h"

namespace Mantis
{
    class Viewport
    {
    public:
        Viewport() :
            m_scale{ 1.0f, 1.0f },
            m_offset{ 0, 0 }
        {
        }

        explicit Viewport(const Vector2Int& size) :
            m_scale{ 1.0f, 1.0f },
            m_size{ size },
            m_offset{ 0, 0 }
        {
        }

        const Vector2& GetScale() const { return m_scale; }

        void SetScale(const Vector2& scale) { m_scale = scale; }

        const eastl::optional<Vector2Int>& GetSize() const { return m_size; }

        void SetSize(const eastl::optional<Vector2Int>& size) { m_size = size; }

        const Vector2Int& GetOffset() const { return m_offset; }

        void SetOffset(const Vector2Int& offset) { m_offset = offset; }

    private:
        Vector2 m_scale;
        eastl::optional<Vector2Int> m_size;
        Vector2Int m_offset;
    };

    class RenderStage
    {
    public:
        /// <summary>
        /// Creates a new render stage.
        /// </summary>
        /// <param name="attachments">The attachments for this stage.</param>
        /// <param name="subpasses">The subpasses for this stage.</param>
        /// <param name="viewport"></param>
        explicit RenderStage(
            eastl::vector<Attachment> attachments = {},
            eastl::vector<Subpass> subpasses = {},
            const Viewport& viewport = Viewport()
        );

        void Update();

        /// <summary>
        /// Rebuilds from the given swapchain.
        /// </summary>
        /// <param name="swapchain">The swapchain to associate with the framebuffers.</param>
        void Rebuild(const Swapchain& swapchain);

        /// <summary>
        /// Finds an attachment by name.
        /// </summary>
        /// <param name="name">The name to find.</param>
        /// <returns>The attachment if found.</returns>
        eastl::optional<Attachment> GetAttachment(const String& name) const;

        /// <summary>
        /// Gets an attachment by index.
        /// </summary>
        /// <param name="binding">The binding index.</param>
        /// <returns>The attachment if found</returns>
        eastl::optional<Attachment> GetAttachment(const uint32_t& binding) const;

        /// <summary>
        /// Gets all attachments for this render stage.
        /// </summary>
        const eastl::vector<Attachment>& GetAttachments() const { return m_attachments; }

        /// <summary>
        /// Gets the types of all subpasses for this render stage.
        /// </summary>
        const eastl::vector<Subpass>& GetSubpasses() const { return m_subpasses; }

        Viewport& GetViewport() { return m_viewport; }

        void SetViewport(const Viewport& viewport) { m_viewport = viewport; }

        /**
         * Gets the render stage viewport.
         * @return The the render stage viewport.
         */
        const RectInt& GetRenderArea() const { return m_renderArea; }

        /**
         * Gets if the width or height has changed between the last update and now.
         * @return If the width or height has changed.
         */
        const bool& IsOutOfDate() const { return m_outOfDate; }

        /// <summary>
        /// Gets the renderpass for this render stage.
        /// </summary>
        const Renderpass* GetRenderpass() const { return m_renderpass.get(); };

        /// <summary>
        /// Gets the depth stencil for this render stage.
        /// </summary>
        const ImageDepth* GetDepthStencil() const { return m_depthStencil.get(); };

        /// <summary>
        /// Gets all the framebuffers for this render stage.
        /// </summary>
        const Framebuffers* GetFramebuffers() const { return m_framebuffers.get(); };

        /// <summary>
        /// Finds a descriptor by name.
        /// </summary>
        /// <param name="name">The name of the descriptor.</param>
        /// <returns>The descriptor, or null if not found.</returns>
        const Descriptor* GetDescriptor(const String& name) const;

        /// <summary>
        /// Gets a framebuffer by index.
        /// </summary>
        /// <param name="index">The index of the framebuffer to get.</param>
        /// <returns>The framebuffer, or the first frambuffer in the index is invalid.</returns>
        const VkFramebuffer& GetFramebuffer(const uint32_t& index) const;

        /// <summary>
        /// Gets the clear values for all the attachments.
        /// </summary>
        const eastl::vector<VkClearValue>& GetClearValues() const { return m_clearValues; }

        /// <summary>
        /// Gets the number of attachments for a subpass.
        /// </summary>
        /// <param name="subpass">The subpass index.</param>
        const uint32_t& GetAttachmentCount(const uint32_t& subpass) const { return m_subpassAttachmentCount[subpass]; }

        /// <summary>
        /// Checks if any attachements store depth.
        /// </summary>
        bool HasDepth() const { return m_depthAttachment.has_value(); }

        /// <summary>
        /// Checks if any attachments belongs to a swapchain.
        /// </summary>
        bool HasSwapchain() const { return m_swapchainAttachment.has_value(); }

        /// <summary>
        /// Checks if a subpass is multisampled.
        /// </summary>
        /// <param name="subpass">The index of the subpass to check.</param>
        bool IsMultisampled(const uint32_t& subpass) const { return m_subpassMultisampled[subpass]; }

    private:
        eastl::vector<Attachment> m_attachments;
        eastl::vector<Subpass> m_subpasses;

        eastl::unique_ptr<Renderpass> m_renderpass;
        eastl::unique_ptr<ImageDepth> m_depthStencil;
        eastl::unique_ptr<Framebuffers> m_framebuffers;

        eastl::map<String, const Descriptor*> m_descriptors;

        eastl::vector<uint32_t> m_subpassAttachmentCount;
        eastl::vector<VkClearValue> m_clearValues;
        eastl::vector<bool> m_subpassMultisampled;
        eastl::optional<Attachment> m_depthAttachment;
        eastl::optional<Attachment> m_swapchainAttachment;

        Viewport m_viewport;
        RectInt m_renderArea;
        bool m_outOfDate;
    };
}
