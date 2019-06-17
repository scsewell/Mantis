#include "stdafx.h"
#include "Image.h"

#include "Renderer/Renderer.h"
#include "Renderer/Buffer/Buffer.h"
//#include "Files/FileSystem.hpp"
//#include "Files/Files.hpp"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

#define LOG_TAG MANTIS_TEXT("Image")

namespace Mantis
{
    static const float ANISOTROPY = 16.0f;

    Image::Image(
        const VkExtent3D& extent,
        const VkImageType& imageType,
        const VkFormat& format,
        const VkSampleCountFlagBits& samples,
        const VkImageTiling& tiling,
        const uint32_t& mipLevels,
        const uint32_t& arrayLayers,
        const VkImageUsageFlags& usage,
        const VkMemoryPropertyFlags& properties
    ) :
        m_image(VK_NULL_HANDLE),
        m_allocator(Renderer::Get()->GetAllocator()),
        m_allocation(VK_NULL_HANDLE),
        m_sampler(VK_NULL_HANDLE),
        m_view(VK_NULL_HANDLE),
        m_extent(extent),
        m_format(format),
        m_samples(samples),
        m_tiling(tiling),
        m_usage(usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        m_mipLevels(mipLevels),
        m_arrayLayers(arrayLayers)
        //m_filter(filter),
        //m_addressMode(addressMode),
        //m_anisotropic(anisotropic),
        //m_layout(layout),
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.requiredFlags = properties;
        allocCreateInfo.memoryTypeBits = 0;
        allocCreateInfo.pool = VK_NULL_HANDLE;

        Image::CreateImage(m_image, m_allocation, m_memoryFlags, m_allocator, allocCreateInfo, m_extent, imageType, m_format, m_samples, m_tiling, m_mipLevels, m_arrayLayers, m_usage);
        //Image::CreateImageView(m_image, m_view, viewType, m_format, imageAspect, m_mipLevels, baseMipLevel, arrayLayers, baseArrayLayer);
        //Image::CreateImageSampler(m_sampler, m_filter, m_addressMode, m_anisotropic, m_mipLevels);
    }

    Image::~Image()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        vkDestroyImageView(*logicalDevice, m_view, nullptr);
        vmaDestroyImage(m_allocator, m_image, m_allocation);
        vkDestroySampler(*logicalDevice, m_sampler, nullptr);
    }

    VkDescriptorSetLayoutBinding Image::GetDescriptorSetLayout(
        const uint32_t& binding,
        const VkDescriptorType& descriptorType,
        const VkShaderStageFlags& stage,
        const uint32_t& count)
    {
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stage;
        layoutBinding.pImmutableSamplers = nullptr;

        return layoutBinding;
    }

    WriteDescriptorSet Image::GetWriteDescriptor(
        const uint32_t& binding,
        const VkDescriptorType& descriptorType,
        const eastl::optional<OffsetSize>& offsetSize) const
    {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = m_sampler;
        imageInfo.imageView = m_view;
        imageInfo.imageLayout = m_layout;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = VK_NULL_HANDLE; // Will be set in the descriptor handler.
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = descriptorType;
        //descriptorWrite.pImageInfo = &imageInfo;

        return WriteDescriptorSet(descriptorWrite, imageInfo);
    }

    eastl::unique_ptr<uint8_t[]> Image::GetContents(
        VkExtent3D& extent,
        const uint32_t& mipLevel,
        const uint32_t& arrayLayer) const
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        extent.width = int32_t(m_extent.width >> mipLevel);
        extent.height = int32_t(m_extent.height >> mipLevel);
        extent.depth = 1;

        VkImage dstImage;
        VkDeviceMemory dstImageMemory;
        CopyImage(m_image, dstImage, dstImageMemory, m_format, m_extent, m_layout, mipLevel, arrayLayer);

        VkImageSubresource dstImageSubresource = {};
        dstImageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        dstImageSubresource.mipLevel = 0;
        dstImageSubresource.arrayLayer = 0;

        VkSubresourceLayout dstSubresourceLayout;
        vkGetImageSubresourceLayout(*logicalDevice, dstImage, &dstImageSubresource, &dstSubresourceLayout);

        auto contents = std::make_unique<uint8_t[]>(dstSubresourceLayout.size);

        void* data;
        vkMapMemory(*logicalDevice, dstImageMemory, dstSubresourceLayout.offset, dstSubresourceLayout.size, 0, &data);
        std::memcpy(contents.get(), data, static_cast<size_t>(dstSubresourceLayout.size));
        vkUnmapMemory(*logicalDevice, dstImageMemory);

        vkFreeMemory(*logicalDevice, dstImageMemory, nullptr);
        vkDestroyImage(*logicalDevice, dstImage, nullptr);

        return contents;
    }

    void Image::SetContents(const uint8_t* contents, const uint32_t& layerCount, const uint32_t& baseArrayLayer)
    {
        // we need to determine the size based on the format of the image
        Buffer stagingBuffer = Buffer(
            GetSize(m_extent, m_format), 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            contents
        );

        CopyBufferToImage(stagingBuffer.GetBuffer(), m_image, m_extent, layerCount, baseArrayLayer);
    }

    uint32_t Image::GetMipLevels(const VkExtent3D & extent)
    {
        //return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, std::max(extent.height, extent.depth)))) + 1);
    }

    bool Image::HasDepth(const VkFormat& format)
    {
        static const eastl::vector<VkFormat> DEPTH_FORMATS = 
        { 
            VK_FORMAT_X8_D24_UNORM_PACK32,
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT
        };
        return eastl::find(DEPTH_FORMATS.begin(), DEPTH_FORMATS.end(), format) != eastl::end(DEPTH_FORMATS);
    }

    bool Image::HasStencil(const VkFormat& format)
    {
        static const eastl::vector<VkFormat> STENCIL_FORMATS =
        {
            VK_FORMAT_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT
        };
        return eastl::find(STENCIL_FORMATS.begin(), STENCIL_FORMATS.end(), format) != eastl::end(STENCIL_FORMATS);
    }

    void Image::CreateImage(
        VkImage& image,
        VmaAllocation& allocation,
        VkMemoryPropertyFlags& memoryFlags,
        const VmaAllocator& allocator,
        const VmaAllocationCreateInfo& allocCreateInfo,
        const VkExtent3D& extent,
        const VkImageType& type,
        const VkFormat& format,
        const VkSampleCountFlagBits& samples,
        const VkImageTiling& tiling,
        const uint32_t& mipLevels,
        const uint32_t& arrayLayers,
        const VkImageUsageFlags& usage
    )
    {
        // create the image
        VkImageCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.flags = arrayLayers == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
        createInfo.imageType = type;
        createInfo.format = format;
        createInfo.extent = extent;
        createInfo.mipLevels = mipLevels;
        createInfo.arrayLayers = arrayLayers;
        createInfo.samples = samples;
        createInfo.tiling = tiling;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationInfo allocInfo;
        if (Renderer::Check(vmaCreateImage(allocator, &createInfo, &allocCreateInfo, &image, &allocation, &allocInfo)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create image.");
        }

        // get the properties of the memory the texture is stored in
        memoryFlags = Renderer::Get()->GetPhysicalDevice()->GetMemoryPropertyFlags(allocInfo.memoryType);
    }

    void Image::CreateImageView(
        VkImageView& imageView,
        const VkImage& image,
        const VkImageViewType& type,
        const VkFormat& format,
        const VkImageAspectFlags& imageAspect,
        const uint32_t& mipLevels,
        const uint32_t& baseMipLevel,
        const uint32_t& layerCount,
        const uint32_t& baseArrayLayer
    )
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = type;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = imageAspect;
        createInfo.subresourceRange.baseMipLevel = baseMipLevel;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
        createInfo.subresourceRange.layerCount = layerCount;

        if (Renderer::Check(vkCreateImageView(*logicalDevice, &createInfo, nullptr, &imageView)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create image view.");
        }
    }

    void Image::CreateImageSampler(
        VkSampler& sampler,
        const VkFilter& filter,
        const VkSamplerAddressMode& addressMode,
        const bool& anisotropic,
        const uint32_t& mipLevels,
        const bool& compare,
        const VkCompareOp& compareOp 
    )
    {
        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkSamplerCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.magFilter = filter;
        createInfo.minFilter = filter;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = addressMode;
        createInfo.addressModeV = addressMode;
        createInfo.addressModeW = addressMode;
        createInfo.mipLodBias = 0.0f;
        createInfo.anisotropyEnable = static_cast<VkBool32>(anisotropic);
        createInfo.maxAnisotropy = (anisotropic && logicalDevice->GetEnabledFeatures().samplerAnisotropy) ? eastl::min(ANISOTROPY, physicalDevice->GetProperties().limits.maxSamplerAnisotropy) : 1.0f;
        createInfo.compareEnable = static_cast<VkBool32>(compare);
        createInfo.compareOp = compareOp;
        createInfo.minLod = 0.0f;
        createInfo.maxLod = static_cast<float>(mipLevels);
        createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        createInfo.unnormalizedCoordinates = VK_FALSE;

        if (Renderer::Check(vkCreateSampler(*logicalDevice, &createInfo, nullptr, &sampler)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create sampler.");
        }
    }

    void Image::CreateMipmaps(
        const VkImage& image,
        const VkExtent3D& extent,
        const VkFormat& format,
        const VkImageLayout& dstImageLayout,
        const uint32_t& mipLevels,
        const uint32_t& baseArrayLayer,
        const uint32_t & layerCount
    )
    {
        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();

        // get device properites for the requested image format
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(*physicalDevice, format, &formatProperties);

        // Mip-chain generation requires support for blit source and destination
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

        CommandBuffer commandBuffer = CommandBuffer();

        for (uint32_t i = 1; i < mipLevels; i++)
        {
            VkImageMemoryBarrier barrier0 = {};
            barrier0.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier0.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier0.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier0.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier0.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier0.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier0.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier0.image = image;
            barrier0.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier0.subresourceRange.baseMipLevel = i - 1;
            barrier0.subresourceRange.levelCount = 1;
            barrier0.subresourceRange.baseArrayLayer = baseArrayLayer;
            barrier0.subresourceRange.layerCount = layerCount;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier0);

            VkImageBlit imageBlit = {};
            imageBlit.srcOffsets[1] = { int32_t(extent.width >> (i - 1)), int32_t(extent.height >> (i - 1)), 1 };
            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.mipLevel = i - 1;
            imageBlit.srcSubresource.baseArrayLayer = baseArrayLayer;
            imageBlit.srcSubresource.layerCount = layerCount;
            imageBlit.dstOffsets[1] = { int32_t(extent.width >> i), int32_t(extent.height >> i), 1 };
            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.mipLevel = i;
            imageBlit.dstSubresource.baseArrayLayer = baseArrayLayer;
            imageBlit.dstSubresource.layerCount = layerCount;
            vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

            VkImageMemoryBarrier barrier1 = {};
            barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier1.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier1.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier1.newLayout = dstImageLayout;
            barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier1.image = image;
            barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier1.subresourceRange.baseMipLevel = i - 1;
            barrier1.subresourceRange.levelCount = 1;
            barrier1.subresourceRange.baseArrayLayer = baseArrayLayer;
            barrier1.subresourceRange.layerCount = layerCount;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);
        }

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = dstImageLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        barrier.subresourceRange.layerCount = layerCount;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        commandBuffer.SubmitIdle();
    }

    void Image::TransitionImageLayout(
        const CommandBuffer& commandBuffer,
        const VkImage& image,
        const VkFormat& format,
        const VkImageLayout& srcImageLayout,
        const VkImageLayout& dstImageLayout,
        const VkImageAspectFlags& imageAspect,
        const uint32_t& mipLevels,
        const uint32_t& baseMipLevel,
        const uint32_t& layerCount,
        const uint32_t& baseArrayLayer
    )
    {
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = srcImageLayout;
        imageMemoryBarrier.newLayout = dstImageLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = imageAspect;
        imageMemoryBarrier.subresourceRange.baseMipLevel = baseMipLevel;
        imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        imageMemoryBarrier.subresourceRange.layerCount = layerCount;

        // source access mask controls actions that have to be finished on the old layout before it will be transitioned to the new layout
        switch (srcImageLayout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                imageMemoryBarrier.srcAccessMask = 0;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                Logger::ErrorT(LOG_TAG, "Unsupported image layout transition source!");
                break;
        }

        // destination access mask controls the dependency for the new image layout
        switch (dstImageLayout)
        {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                if (imageMemoryBarrier.srcAccessMask == 0)
                {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                Logger::ErrorT(LOG_TAG, "Unsupported image layout transition destination!");
                break;
        }

        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void Image::InsertImageMemoryBarrier(
        const CommandBuffer& commandBuffer,
        const VkImage& image, 
        const VkAccessFlags& srcAccessMask,
        const VkAccessFlags& dstAccessMask,
        const VkImageLayout& oldImageLayout,
        const VkImageLayout& newImageLayout,
        const VkPipelineStageFlags& srcStageMask,
        const VkPipelineStageFlags& dstStageMask,
        const VkImageAspectFlags& imageAspect,
        const uint32_t& mipLevels,
        const uint32_t& baseMipLevel, 
        const uint32_t& layerCount,
        const uint32_t& baseArrayLayer
    )
    {
        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = imageAspect;
        imageMemoryBarrier.subresourceRange.baseMipLevel = baseMipLevel;
        imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        imageMemoryBarrier.subresourceRange.layerCount = layerCount;
        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void Image::CopyBufferToImage(
        const CommandBuffer& commandBuffer,
        const VkBuffer& buffer,
        const VkImage& image,
        const VkExtent3D& extent,
        const uint32_t& layerCount,
        const uint32_t& baseArrayLayer
    )
    {
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = baseArrayLayer;
        region.imageSubresource.layerCount = layerCount;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = extent;

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    bool Image::CopyImage(
        const VkImage& srcImage,
        VkImage& dstImage,
        VkDeviceMemory& dstImageMemory,
        const VkFormat& srcFormat,
        const VkExtent3D& extent,
        const VkImageLayout& srcImageLayout,
        const uint32_t& mipLevel, 
        const uint32_t& arrayLayer
    )
    {
        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();
        auto surface = Renderer::Get()->GetSurface();

        // Checks blit swapchain support.
        bool supportsBlit = true;
        VkFormatProperties formatProperties;

        // Check if the device supports blitting from optimal images (the swapchain images are in optimal format).
        vkGetPhysicalDeviceFormatProperties(*physicalDevice, surface->GetFormat().format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures& VK_FORMAT_FEATURE_BLIT_SRC_BIT))
        {
            Logger::WarningT(LOG_TAG, "Device does not support blitting from optimal tiled images, using copy instead of blit!");
            supportsBlit = false;
        }

        // Check if the device supports blitting to linear images.
        vkGetPhysicalDeviceFormatProperties(*physicalDevice, srcFormat, &formatProperties);

        if (!(formatProperties.linearTilingFeatures& VK_FORMAT_FEATURE_BLIT_DST_BIT))
        {
            Logger::WarningT(LOG_TAG, "Device does not support blitting to linear tiled images, using copy instead of blit!");
            supportsBlit = false;
        }

        CreateImage(dstImage, dstImageMemory, extent, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1, 1, VK_IMAGE_TYPE_2D);

        // Do the actual blit from the swapchain image to our host visible destination image.
        CommandBuffer commandBuffer = CommandBuffer();

        // Transition destination image to transfer destination layout.
        InsertImageMemoryBarrier(commandBuffer, dstImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1, 0);

        // Transition image from previous usage to transfer source layout
        InsertImageMemoryBarrier(commandBuffer, srcImage, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, srcImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, mipLevel, 1, arrayLayer);

        // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
        if (supportsBlit)
        {
            // Define the region to blit (we will blit the whole swapchain image).
            VkOffset3D blitSize = { static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), static_cast<int32_t>(extent.depth) };
            VkImageBlit imageBlitRegion = {};
            imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlitRegion.srcSubresource.mipLevel = mipLevel;
            imageBlitRegion.srcSubresource.baseArrayLayer = arrayLayer;
            imageBlitRegion.srcSubresource.layerCount = 1;
            imageBlitRegion.srcOffsets[1] = blitSize;
            imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlitRegion.dstSubresource.mipLevel = 0;
            imageBlitRegion.dstSubresource.baseArrayLayer = 0;
            imageBlitRegion.dstSubresource.layerCount = 1;
            imageBlitRegion.dstOffsets[1] = blitSize;
            vkCmdBlitImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);
        }
        else
        {
            // Otherwise use image copy (requires us to manually flip components).
            VkImageCopy imageCopyRegion = {};
            imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.srcSubresource.mipLevel = mipLevel;
            imageCopyRegion.srcSubresource.baseArrayLayer = arrayLayer;
            imageCopyRegion.srcSubresource.layerCount = 1;
            imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.dstSubresource.mipLevel = 0;
            imageCopyRegion.dstSubresource.baseArrayLayer = 0;
            imageCopyRegion.dstSubresource.layerCount = 1;
            imageCopyRegion.extent = extent;
            vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
        }

        // Transition destination image to general layout, which is the required layout for mapping the image memory later on.
        InsertImageMemoryBarrier(commandBuffer, dstImage, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1, 0);

        // Transition back the image after the blit is done.
        InsertImageMemoryBarrier(commandBuffer, srcImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageLayout,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, mipLevel, 1, arrayLayer);

        commandBuffer.SubmitIdle();

        return supportsBlit;
    }

    VkDeviceSize Image::GetSize(const VkExtent3D& extents, const VkFormat& format)
    {
        uint32_t texels = extents.width * extents.height * extents.depth;

        // convert from texels to blocks, from blocks to bytes
        switch (format)
        {
            // color texture formats
            case VK_FORMAT_R4G4_UNORM_PACK8:
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SNORM:
            case VK_FORMAT_R8_USCALED:
            case VK_FORMAT_R8_SSCALED:
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R8_SRGB:
                return (texels / 1) * 1;
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SNORM:
            case VK_FORMAT_R8G8_USCALED:
            case VK_FORMAT_R8G8_SSCALED:
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R8G8_SINT:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R16_UNORM:
            case VK_FORMAT_R16_SNORM:
            case VK_FORMAT_R16_USCALED:
            case VK_FORMAT_R16_SSCALED:
            case VK_FORMAT_R16_UINT:
            case VK_FORMAT_R16_SINT:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R10X6_UNORM_PACK16:
            case VK_FORMAT_R12X4_UNORM_PACK16:
                return (texels / 1) * 2;
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SNORM:
            case VK_FORMAT_R8G8B8_USCALED:
            case VK_FORMAT_R8G8B8_SSCALED:
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_R8G8B8_SINT:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_UNORM:
            case VK_FORMAT_B8G8R8_SNORM:
            case VK_FORMAT_B8G8R8_USCALED:
            case VK_FORMAT_B8G8R8_SSCALED:
            case VK_FORMAT_B8G8R8_UINT:
            case VK_FORMAT_B8G8R8_SINT:
            case VK_FORMAT_B8G8R8_SRGB:
                return (texels / 1) * 3;
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SNORM:
            case VK_FORMAT_R8G8B8A8_USCALED:
            case VK_FORMAT_R8G8B8A8_SSCALED:
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SNORM:
            case VK_FORMAT_B8G8R8A8_USCALED:
            case VK_FORMAT_B8G8R8A8_SSCALED:
            case VK_FORMAT_B8G8R8A8_UINT:
            case VK_FORMAT_B8G8R8A8_SINT:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            case VK_FORMAT_R16G16_UNORM:
            case VK_FORMAT_R16G16_SNORM:
            case VK_FORMAT_R16G16_USCALED:
            case VK_FORMAT_R16G16_SSCALED:
            case VK_FORMAT_R16G16_UINT:
            case VK_FORMAT_R16G16_SINT:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R32_SINT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
            case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
                return (texels / 1) * 4;
            case VK_FORMAT_G8B8G8R8_422_UNORM:
                return (texels / 1) * 4;
            case VK_FORMAT_B8G8R8G8_422_UNORM:
                return (texels / 1) * 4;
            case VK_FORMAT_R16G16B16_UNORM:
            case VK_FORMAT_R16G16B16_SNORM:
            case VK_FORMAT_R16G16B16_USCALED:
            case VK_FORMAT_R16G16B16_SSCALED:
            case VK_FORMAT_R16G16B16_UINT:
            case VK_FORMAT_R16G16B16_SINT:
            case VK_FORMAT_R16G16B16_SFLOAT:
                return (texels / 1) * 6;
            case VK_FORMAT_R16G16B16A16_UNORM:
            case VK_FORMAT_R16G16B16A16_SNORM:
            case VK_FORMAT_R16G16B16A16_USCALED:
            case VK_FORMAT_R16G16B16A16_SSCALED:
            case VK_FORMAT_R16G16B16A16_UINT:
            case VK_FORMAT_R16G16B16A16_SINT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R32G32_SINT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R64_UINT:
            case VK_FORMAT_R64_SINT:
            case VK_FORMAT_R64_SFLOAT:
                return (texels / 1) * 8;
            case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
            case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
            case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
            case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
            case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
            case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
            case VK_FORMAT_G16B16G16R16_422_UNORM:
            case VK_FORMAT_B16G16R16G16_422_UNORM:
                return (texels / 1) * 8;
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R32G32B32_SINT:
            case VK_FORMAT_R32G32B32_SFLOAT:
                return (texels / 1) * 12;
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R32G32B32A32_SINT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R64G64_UINT:
            case VK_FORMAT_R64G64_SINT:
            case VK_FORMAT_R64G64_SFLOAT:
                return (texels / 1) * 16;
            case VK_FORMAT_R64G64B64_UINT:
            case VK_FORMAT_R64G64B64_SINT:
            case VK_FORMAT_R64G64B64_SFLOAT:
                return (texels / 1) * 24;
            case VK_FORMAT_R64G64B64A64_UINT:
            case VK_FORMAT_R64G64B64A64_SINT:
            case VK_FORMAT_R64G64B64A64_SFLOAT:
                return (texels / 1) * 32;
            // compressed texture formats
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
                return (texels / 16) * 8;
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
                return (texels / 16) * 8;
            case VK_FORMAT_BC2_UNORM_BLOCK:
            case VK_FORMAT_BC2_SRGB_BLOCK:
                return (texels / 16) * 16;
            case VK_FORMAT_BC3_UNORM_BLOCK:
            case VK_FORMAT_BC3_SRGB_BLOCK:
                return (texels / 16) * 16;
            case VK_FORMAT_BC4_UNORM_BLOCK:
            case VK_FORMAT_BC4_SNORM_BLOCK:
                return (texels / 16) * 8;
            case VK_FORMAT_BC5_UNORM_BLOCK:
            case VK_FORMAT_BC5_SNORM_BLOCK:
                return (texels / 16) * 16;
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
                return (texels / 16) * 16;
            case VK_FORMAT_BC7_UNORM_BLOCK:
            case VK_FORMAT_BC7_SRGB_BLOCK:
                return (texels / 16) * 16;
            // depth texture formats
            case VK_FORMAT_S8_UINT:
                return (texels / 1) * 1;
            case VK_FORMAT_D16_UNORM:
                return (texels / 1) * 2;
            case VK_FORMAT_D16_UNORM_S8_UINT:
                return (texels / 1) * 3;
            case VK_FORMAT_D24_UNORM_S8_UINT:
                return (texels / 1) * 4;
            case VK_FORMAT_X8_D24_UNORM_PACK32:
                return (texels / 1) * 4;
            case VK_FORMAT_D32_SFLOAT:
                return (texels / 1) * 4;
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return (texels / 1) * 5;
            default:
                Logger::ErrorTF(LOG_TAG, "Cannot compute reqired image size, unsupported format: &u", static_cast<uint32_t>(format));
                break;
        }
    }
}