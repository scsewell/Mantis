#pragma once

#include "Mantis.h"

#include "Image.h"
#include "Renderer/Utils/Nameable.h"

namespace Mantis
{
	/// <summary>
	/// The creation options for an image view.
	/// </summary>
    struct ImageViewCreateInfo
    {
        // uses the format of the associated image by default
        VkFormat format = VK_FORMAT_UNDEFINED;
        // assumes the type from the associated image by default
        VkImageViewType type = VK_IMAGE_VIEW_TYPE_RANGE_SIZE;
        VkComponentMapping swizzle =
        {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
        };
        uint32_t baseLevel = 0;
        uint32_t levels = VK_REMAINING_MIP_LEVELS;
        uint32_t baseLayer = 0;
        uint32_t layers = VK_REMAINING_ARRAY_LAYERS;
    };

    /// <summary>
    /// Manages an image view.
    /// </summary>
    class ImageView
        : public NonCopyable
        , public Nameable
    {
    public:
        /// <summary>
        /// Creates a new image view.
        /// </summary>
        /// <param name="image">The image associated with this view.</param>
        /// <param name="createInfo">The creation parameters.</param>
        explicit ImageView(
            const Image* image,
            const ImageViewCreateInfo& createInfo
        );

        /// <summary>
        /// Destroys the image view.
        /// </summary>
        virtual ~ImageView();

        /// <summary>
        /// Sets the name of this instance.
        /// </summary>
        void SetName(const String& name);

    private:
        static VkImageViewType GetImageViewType(const Image* image, const ImageViewCreateInfo& createInfo);

        VkImageView m_view;
    };
}
