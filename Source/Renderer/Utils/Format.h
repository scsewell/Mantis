#pragma once

#include "Mantis.h"

namespace Mantis
{
    class Format
    {
    public:
        /// <summary>
        /// Checks if a format is stored in the sRGB color space.
        /// </summary>
        /// <param name="format">The format to check.</param>
        static inline bool IsSrgb(VkFormat format)
        {
            switch (format)
            {
                case VK_FORMAT_R8_SRGB:
                case VK_FORMAT_R8G8_SRGB:
                case VK_FORMAT_R8G8B8_SRGB:
                case VK_FORMAT_B8G8R8_SRGB:
                case VK_FORMAT_R8G8B8A8_SRGB:
                case VK_FORMAT_B8G8R8A8_SRGB:
                case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                    return true;
                default:
                    return false;
            }
        }

        /// <summary>
        /// Checks if a format has a depth aspect.
        /// </summary>
        /// <param name="format">The format to check.</param>
        static inline bool HasDepth(const VkFormat& format)
        {
            switch (format)
            {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    return true;
                default:
                    return false;
            }
        }

        /// <summary>
        /// Checks if a format has a stencil aspect.
        /// </summary>
        /// <param name="format">The format to check.</param>
        static inline bool HasStencil(const VkFormat& format)
        {
            switch (format)
            {
                case VK_FORMAT_S8_UINT:
                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    return true;
                default:
                    return false;
            }
        }

        /// <summary>
        /// Checks if a format has a depth or stencil aspect.
        /// </summary>
        /// <param name="format">The format to check.</param>
        static inline bool HasDepthOrStencil(const VkFormat& format)
        {
            return HasDepth(format) || HasStencil(format);
        }

        /// <summary>
        /// Gets the image aspect suitable for a format.
        /// </summary>
        /// <param name="format">The format to get the aspect of.</param>
        static inline VkImageAspectFlags GetImageAspect(const VkFormat& format)
        {
            switch (format)
            {
                case VK_FORMAT_UNDEFINED:
                    return 0;

                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                    return VK_IMAGE_ASPECT_DEPTH_BIT;

                case VK_FORMAT_S8_UINT:
                    return VK_IMAGE_ASPECT_STENCIL_BIT;

                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

                default:
                    return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }
    };
}
