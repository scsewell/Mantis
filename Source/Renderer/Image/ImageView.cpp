#include "stdafx.h"
#include "ImageView.h"

#include "Renderer/Renderer.h"
#include "Renderer/Utils/Format.h"
#include <cassert>

#define LOG_TAG MANTIS_TEXT("ImageView")

namespace Mantis
{
    ImageView::ImageView(
        const Image* image,
        const ImageViewCreateInfo& createInfo
    )
        : m_view(VK_NULL_HANDLE)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        // ensure the image supports having a view
        if (HAS_NO_FLAG(image->GetUsage(), 
            VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        ))
        {
            Logger::ErrorTF(LOG_TAG, "Image view cannot be created for image with usage: %u!", image->GetUsage());
        }

        VkFormat format = createInfo.format != VK_FORMAT_UNDEFINED ? createInfo.format : image->GetFormat();

        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = image->GetImage();
        info.format = format;
        info.components = createInfo.swizzle;
        info.subresourceRange.aspectMask = Format::GetImageAspect(format);
        info.subresourceRange.baseMipLevel = createInfo.baseLevel;
        info.subresourceRange.baseArrayLayer = createInfo.baseLayer;

        if (createInfo.type == VK_IMAGE_VIEW_TYPE_RANGE_SIZE)
        {
            info.viewType = GetImageViewType(image, createInfo);
        }
        else
        {
            info.viewType = createInfo.type;
        }

        if (createInfo.levels == VK_REMAINING_MIP_LEVELS)
        {
            info.subresourceRange.levelCount = image->GetLevelCount() - createInfo.baseLevel;
        }
        else
        {
            info.subresourceRange.levelCount = createInfo.levels;
        }

        if (createInfo.layers == VK_REMAINING_ARRAY_LAYERS)
        {
            info.subresourceRange.layerCount = image->GetLayerCount() - createInfo.baseLayer;
        }
        else
        {
            info.subresourceRange.layerCount = createInfo.layers;
        }

        if (Renderer::Check(vkCreateImageView(*logicalDevice, &info, nullptr, &m_view)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create image view!");
        }
    }

    ImageView::~ImageView()
    {
        Renderer::Get()->DestroyImageView(m_view);
    }

    void ImageView::SetName(const String& name)
    {
        SetDebugName(name, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_view);
    }

    VkImageViewType ImageView::GetImageViewType(const Image* image, const ImageViewCreateInfo& createInfo)
    {
        uint32_t layers = createInfo.layers;

        if (layers == VK_REMAINING_ARRAY_LAYERS)
        {
            layers = image->GetLayerCount() - createInfo.baseLayer;
        }

        switch (image->GetType())
        {
            case VK_IMAGE_TYPE_1D:
                assert(image->GetExtents().width >= 1);
                assert(image->GetExtents().height == 1);
                assert(image->GetExtents().depth == 1);
                assert(image->GetSamples() == VK_SAMPLE_COUNT_1_BIT);

                if (layers > 1)
                {
                    return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                }
                else
                {
                    return VK_IMAGE_VIEW_TYPE_1D;
                }

            case VK_IMAGE_TYPE_2D:
                assert(image->GetExtents().width >= 1);
                assert(image->GetExtents().height >= 1);
                assert(image->GetExtents().depth == 1);

                if (image->IsCubeCompatible())
                {
                    if (layers > 6)
                    {
                        return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                    }
                    else
                    {
                        return VK_IMAGE_VIEW_TYPE_CUBE;
                    }
                }
                else
                {
                    if (layers > 1)
                    {
                        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                    }
                    else
                    {
                        return VK_IMAGE_VIEW_TYPE_2D;
                    }
                }

            case VK_IMAGE_TYPE_3D:
                assert(image->GetExtents().width >= 1);
                assert(image->GetExtents().height >= 1);
                assert(image->GetExtents().depth >= 1);
                return VK_IMAGE_VIEW_TYPE_3D;

            default:
                Logger::ErrorTF(LOG_TAG, "Unsupported image type: %u", image->GetType());
                return VK_IMAGE_VIEW_TYPE_RANGE_SIZE;
        }
    }
}
