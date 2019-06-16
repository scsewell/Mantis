#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Class that represents an attachment in a renderpass.
    /// </summary>
    class Attachment
    {
    public:
        /// <summary>
        /// The type of attachment.
        /// </summary>
        enum struct Type
        {
            Image,
            Depth,
            Swapchain,
        };

        /// <summary>
        /// How an attachment should be treated when loaded.
        /// </summary>
        enum struct LoadOp
        {
            Load = VK_ATTACHMENT_LOAD_OP_LOAD,
            Clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
            DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        };

        /// <summary>
        /// How an attachment should be treaded when stored.
        /// </summary>
        enum struct StoreOp
        {
            Store = VK_ATTACHMENT_STORE_OP_STORE,
            DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        };

        /// <summary>
        /// Creates a new attachment that represents a object in the render pipeline.
        /// </summary>
        /// <param name="binding"The index the attachment is bound to in the renderpass.></param>
        /// <param name="name">The unique name given to the object for all renderpasses.</param>
        /// <param name="type">The attachment type this represents.</param>
        /// <param name="loadOp">Gets how the attachment should be treated when loaded.</param>
        /// <param name="storeOp">Gets how the attachment should be treated when stored.</param>
        /// <param name="multisampled">Should this attachment be multisampled.</param>
        /// <param name="format">The format that will be created (only applies to image attachements).</param>
        /// <param name="clearColor">The color to clear to before rendering to it.</param>
        Attachment(
            const uint32_t& binding,
            String name,
            const Type& type,
            const LoadOp& loadOp,
            const StoreOp& storeOp,
            const bool& multisampled = false,
            const VkFormat& format = VK_FORMAT_R8G8B8A8_UNORM,
            const Color& clearColor = Color::Black()
        ) :
            m_binding(binding),
            m_name(eastl::move(name)),
            m_type(type),
            m_loadOp(loadOp),
            m_storeOp(storeOp),
            m_multisampled(multisampled),
            m_format(format),
            m_clearColor(clearColor)
        {
        }

        /// <summary>
        /// The index the attachment is bound to in the renderpass.
        /// </summary>
        const uint32_t& GetBinding() const { return m_binding; }

        /// <summary>
        /// The unique name given to the object for all renderpasses.
        /// </summary>
        const String& GetName() const { return m_name; }

        /// <summary>
        /// The type of attachment.
        /// </summary>
        const Type& GetType() const { return m_type; }

        /// <summary>
        /// Gets how the attachment should be treated when loaded.
        /// </summary>
        const LoadOp& GetLoadOp() const { return m_loadOp; }

        /// <summary>
        /// Gets how the attachment should be treated when stored.
        /// </summary>
        const StoreOp& GetStoreOp() const { return m_storeOp; }

        /// <summary>
        /// Should this attachment be multisampled.
        /// </summary>
        const bool& IsMultisampled() const { return m_multisampled; }

        /// <summary>
        /// The format that will be created (only applies to image attachements).
        /// </summary>
        const VkFormat& GetFormat() const { return m_format; }

        /// <summary>
        /// The color to clear to before rendering to it.
        /// </summary>
        const Color& GetClearColor() const { return m_clearColor; }

    private:
        uint32_t m_binding;
        String m_name;
        Type m_type;
        LoadOp m_loadOp;
        StoreOp m_storeOp;
        bool m_multisampled;
        VkFormat m_format;
        Color m_clearColor;
    };
}
