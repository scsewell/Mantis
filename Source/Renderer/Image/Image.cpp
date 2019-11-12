#include "stdafx.h"
#include "Image.h"

#include "Renderer/Renderer.h"
#include "Renderer/Buffer/Buffer.h"

#define LOG_TAG MANTIS_TEXT("Image")

namespace Mantis
{
    Image::Image() :
        m_image(VK_NULL_HANDLE),
        m_allocator(Renderer::Get()->GetAllocator()),
        m_allocation(VK_NULL_HANDLE),
        m_memoryFlags(0)
    {
    }

    Image::~Image()
    {
		Renderer::Get()->DestroyImage(m_image, m_allocation);
    }

    void Image::Create(
        const VkMemoryPropertyFlags& properties,
        const VkImageUsageFlags& usage,
        const VkImageCreateFlags& flags,
        const VkImageType& type,
        const VkImageViewType& viewType,
        const VkExtent3D& extent,
        const VkFormat& format,
        const VkImageTiling& tiling,
        const VkSampleCountFlagBits& samples,
        const uint32_t& mipLevels,
        const uint32_t& arrayLayers,
        const VkFilter& filter,
        const VkSamplerAddressMode& addressMode,
        const bool& anisotropic,
        const bool& compare,
        const VkCompareOp& compareOp)
    {
        m_usage = usage;
        m_extent = extent;
        m_format = format;
        m_samples = samples;
        m_mipLevels = mipLevels;
        m_arrayLayers = arrayLayers;

        m_filter = filter;
        m_addressMode = addressMode;
        m_anisotropic = anisotropic;

        CreateImage(properties, flags, type, tiling);
        CreateView(viewType);
        CreateSampler(compare, compareOp);
    }

    void Image::CreateImage(
        const VkMemoryPropertyFlags& properties,
        const VkImageCreateFlags& flags,
        const VkImageType& type,
        const VkImageTiling& tiling)
    {
        // describe the image
        VkImageCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.flags = flags;
        createInfo.imageType = type;
        createInfo.format = m_format;
        createInfo.extent = m_extent;
        createInfo.mipLevels = m_mipLevels;
        createInfo.arrayLayers = m_arrayLayers;
        createInfo.samples = m_samples;
        createInfo.tiling = tiling;
        createInfo.usage = m_usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // describe the memory to use
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.requiredFlags = properties;
        allocCreateInfo.memoryTypeBits = 0;
        allocCreateInfo.pool = VK_NULL_HANDLE;

        // create the image
        VmaAllocationInfo allocInfo;
        if (Renderer::Check(vmaCreateImage(m_allocator, &createInfo, &allocCreateInfo, &m_image, &m_allocation, &allocInfo)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create image.");
        }

        // get the properties of the memory the texture is stored in
        m_memoryFlags = Renderer::Get()->GetPhysicalDevice()->GetMemoryPropertyFlags(allocInfo.memoryType);
    }

    void Image::CreateView(const VkImageViewType& viewType)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_image;
        createInfo.viewType = viewType;
        createInfo.format = m_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = GetImageAspect(m_format);
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = m_mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = m_arrayLayers;

        if (Renderer::Check(vkCreateImageView(*logicalDevice, &createInfo, nullptr, &m_view)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create view.");
        }
    }

    void Image::SetName(const String& name)
    {
        SetDebugName(name, VK_OBJECT_TYPE_IMAGE, (uint64_t)m_image);
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
        //VkDescriptorImageInfo imageInfo = {};
        //imageInfo.sampler = m_sampler;
        //imageInfo.imageView = m_view;
        //imageInfo.imageLayout = m_layout;

        //VkWriteDescriptorSet descriptorWrite = {};
        //descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //descriptorWrite.dstSet = VK_NULL_HANDLE; // Will be set in the descriptor handler.
        //descriptorWrite.dstBinding = binding;
        //descriptorWrite.dstArrayElement = 0;
        //descriptorWrite.descriptorCount = 1;
        //descriptorWrite.descriptorType = descriptorType;
        //descriptorWrite.pImageInfo = &imageInfo;

        //return WriteDescriptorSet(descriptorWrite, imageInfo);
    }

    eastl::unique_ptr<uint8_t[]> Image::GetContents(
        uint32_t& size,
        VkExtent3D& extent,
        const uint32_t& mipLevel,
        const uint32_t& arrayLayer) const
    {
        //auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        //if (arrayLayer >= m_arrayLayers)
        //{
        //    Logger::ErrorTF(LOG_TAG, "Cannot get contents of layer %u, image only has %u layers!", arrayLayer, m_arrayLayers);
        //    return nullptr;
        //}

        //uint32_t level = eastl::min(mipLevel, m_mipLevels - 1);

        //extent.width = eastl::max(m_extent.width >> level, 1u);
        //extent.height = eastl::max(m_extent.height >> level, 1u);
        //extent.depth = 1;

        //CommandBuffer cmd = CommandBuffer(QueueType::);

        //InsertImageMemoryBarrier(commandBuffer, dstImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1, 0);


        //VkImage dstImage;
        //VkDeviceMemory dstImageMemory;
        //CopyImage(m_image, dstImage, dstImageMemory, m_format, m_extent, m_layout, level, arrayLayer);

        //VkImageSubresource dstImageSubresource = {};
        //dstImageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //dstImageSubresource.mipLevel = 0;
        //dstImageSubresource.arrayLayer = 0;

        //VkSubresourceLayout dstSubresourceLayout;
        //vkGetImageSubresourceLayout(*logicalDevice, dstImage, &dstImageSubresource, &dstSubresourceLayout);

        //auto contents = std::make_unique<uint8_t[]>(dstSubresourceLayout.size);

        //void* data;
        //vkMapMemory(*logicalDevice, dstImageMemory, dstSubresourceLayout.offset, dstSubresourceLayout.size, 0, &data);
        //std::memcpy(contents.get(), data, static_cast<size_t>(dstSubresourceLayout.size));
        //vkUnmapMemory(*logicalDevice, dstImageMemory);

        //vkFreeMemory(*logicalDevice, dstImageMemory, nullptr);
        //vkDestroyImage(*logicalDevice, dstImage, nullptr);

        //return contents;
    }

    void Image::SetContents(
        const uint8_t* contents,
        const uint32_t& baseMipLevel,
        const uint32_t& mipLevelCount,
        const uint32_t& baseLayer,
        const uint32_t& layerCount)
    {
        uint32_t endMip = baseMipLevel + mipLevelCount;
        uint32_t endLayer = baseLayer + layerCount;

        // check that the contents to set exist
        if (endMip > m_mipLevels)
        {
            Logger::ErrorTF(LOG_TAG, "Cannot set contents of mip levels %u to %u, image only has %u mip levels!", baseMipLevel, endMip - 1, m_mipLevels);
            return;
        }
        if (endLayer > m_arrayLayers)
        {
            Logger::ErrorTF(LOG_TAG, "Cannot set contents of layers %u to %u, image only has %u layers!", baseLayer, endLayer - 1, m_arrayLayers);
            return;
        }

        // prepare to copy all layers and mips
        eastl::vector<VkBufferImageCopy> regions;
        uint32_t size = 0;

        for (uint32_t layer = baseLayer; layer < endLayer; layer++)
        {
            for (uint32_t mip = baseMipLevel; mip < endMip; mip++)
            {
                // compute the resolution of the mip map
                VkExtent3D mipExtent = {};
                mipExtent.width  = eastl::max(m_extent.width  >> mip, 1u);
                mipExtent.height = eastl::max(m_extent.height >> mip, 1u);
                mipExtent.depth  = eastl::max(m_extent.depth  >> mip, 1u);

                // determine the the mapping between regions to copy between
                VkBufferImageCopy region = {};
                region.bufferOffset = size;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = GetImageAspect(m_format);
                region.imageSubresource.mipLevel = mip;
                region.imageSubresource.baseArrayLayer = layer;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = mipExtent;

                regions.push_back(region);

                // keep track of the total size of the data to copy
                size += static_cast<uint32_t>(GetSize(mipExtent, m_format));
            }
        }

        // get a staging buffer of suitable size and copy in the contents
        Buffer stagingBuffer = Buffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            contents
        );

        // check if a separate transfer queue should be used
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        auto graphicsFamily = logicalDevice->GetGraphicsFamily();
        auto transferFamily = logicalDevice->GetTransferFamily();

        bool isUnifiedQueue = graphicsFamily == transferFamily;

        // create a command buffer to do the copy with
        CommandBuffer cmd = CommandBuffer(isUnifiedQueue ? QueueType::Graphics : QueueType::Transfer);

        // transition the image to a layout suitable to copy to
        TransitionImageLayout(cmd,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        // copy the contents to the image
        vkCmdCopyBufferToImage(cmd, stagingBuffer.GetBuffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data());

        if (isUnifiedQueue)
        {
            TransitionImageLayout(cmd,
                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            cmd.SubmitIdle();
        }
        else
        {
            TransitionImageLayout(cmd,
                transferFamily, graphicsFamily,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            
            cmd.SubmitIdle();

            CommandBuffer cmd = CommandBuffer(QueueType::Graphics);

            TransitionImageLayout(cmd,
                transferFamily, graphicsFamily,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );

            cmd.SubmitIdle();
        }
    }

    void Image::GenerateMipmaps(const CommandBuffer& commandBuffer)
    {
        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();

        // check that blitting is supported for the image type
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(*physicalDevice, m_format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            Logger::ErrorT(LOG_TAG, "Device does not support linear blitting!");
            return;
        }

        VkImageAspectFlags aspect = GetImageAspect(m_format);

        // generate all the mip maps
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = aspect;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = m_arrayLayers;

        for (uint32_t i = 1; i < m_mipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit = {};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { int32_t(eastl::max(m_extent.width >> (i - 1), 1u)), int32_t(eastl::max(m_extent.height >> (i - 1), 1u)), int32_t(eastl::max(m_extent.depth >> (i - 1), 1u)) };
            blit.srcSubresource.aspectMask = aspect;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = m_arrayLayers;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { int32_t(eastl::max(m_extent.width >> i, 1u)), int32_t(eastl::max(m_extent.height >> i, 1u)), int32_t(eastl::max(m_extent.depth >> i, 1u)) };
            blit.dstSubresource.aspectMask = aspect;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = m_arrayLayers;

            vkCmdBlitImage(commandBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        }

        barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void Image::TransitionImageLayout(
        const CommandBuffer& commandBuffer,
        const uint32_t& srcQueueFamilyIndex,
        const uint32_t& dstQueueFamilyIndex,
        const VkImageLayout& srcImageLayout,
        const VkImageLayout& dstImageLayout)
    {
        // check if there is a resourece ownership transition to a different queue
        bool isQueueTransfer = srcQueueFamilyIndex != dstQueueFamilyIndex;
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();
        uint32_t queueIndex = logicalDevice->GetQueueFamilyIndex(commandBuffer.GetQueueType());

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = srcImageLayout;
        barrier.newLayout = dstImageLayout;
        barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = GetImageAspect(m_format);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = m_arrayLayers;

        VkPipelineStageFlags srcStage;
        if (isQueueTransfer && queueIndex == dstQueueFamilyIndex)
        {
            barrier.srcAccessMask = 0;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        else
        {
            switch (srcImageLayout)
            {
                case VK_IMAGE_LAYOUT_UNDEFINED:
                    barrier.srcAccessMask = 0;
                    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                    break;
                case VK_IMAGE_LAYOUT_PREINITIALIZED:
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                    srcStage = VK_PIPELINE_STAGE_HOST_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    break;
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    break;
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    break;
                default:
                    Logger::ErrorT(LOG_TAG, "Unsupported image layout transition source!");
                    break;
            }
        }

        VkPipelineStageFlags dstStage;
        if (isQueueTransfer && queueIndex == srcQueueFamilyIndex)
        {
            barrier.dstAccessMask = 0;
            dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        else
        {
            switch (dstImageLayout)
            {
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    break;
                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    break;
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    break;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                    break;
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    break;
                default:
                    Logger::ErrorT(LOG_TAG, "Unsupported image layout transition destination!");
                    break;
            }
        }

        vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void Image::InsertMemoryBarrier(
        const CommandBuffer& commandBuffer,
        const uint32_t& srcQueueFamilyIndex,
        const uint32_t& dstQueueFamilyIndex,
        const VkAccessFlags& srcAccessMask,
        const VkAccessFlags& dstAccessMask,
        const VkImageLayout& oldImageLayout,
        const VkImageLayout& newImageLayout,
        const VkPipelineStageFlags& srcStageMask,
        const VkPipelineStageFlags& dstStageMask)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.oldLayout = oldImageLayout;
        barrier.newLayout = newImageLayout;
        barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = GetImageAspect(m_format);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = m_arrayLayers;

        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    bool Image::CopyImage(
        const VkImage& srcImage,
        VkImage& dstImage,
        VkDeviceMemory& dstImageMemory,
        const VkFormat& srcFormat,
        const VkExtent3D& extent,
        const VkImageLayout& srcImageLayout)
    {
        //auto physicalDevice = Renderer::Get()->GetPhysicalDevice();

        //// checks blit support
        //bool supportsBlit = true;
        //VkFormatProperties formatProperties;

        //// check if the device supports blitting from optimal images
        //vkGetPhysicalDeviceFormatProperties(*physicalDevice, m_format, &formatProperties);

        //if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
        //{
        //    Logger::WarningT(LOG_TAG, "Device does not support blitting from optimal tiled images, using copy instead of blit!");
        //    supportsBlit = false;
        //}

        //// check if the device supports blitting to linear images
        //vkGetPhysicalDeviceFormatProperties(*physicalDevice, srcFormat, &formatProperties);

        //if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
        //{
        //    Logger::WarningT(LOG_TAG, "Device does not support blitting to linear tiled images, using copy instead of blit!");
        //    supportsBlit = false;
        //}

        //CreateImage(dstImage, dstImageMemory, extent, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR,
        //    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1, 1, VK_IMAGE_TYPE_2D);

        //// Do the actual blit from the swapchain image to our host visible destination image.
        //CommandBuffer commandBuffer = CommandBuffer();

        //// Transition destination image to transfer destination layout.
        //InsertImageMemoryBarrier(commandBuffer, dstImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1, 0);

        //// Transition image from previous usage to transfer source layout
        //InsertImageMemoryBarrier(commandBuffer, srcImage, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, srcImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, mipLevel, 1, arrayLayer);

        //// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
        //if (supportsBlit)
        //{
        //    // Define the region to blit (we will blit the whole swapchain image).
        //    VkOffset3D blitSize = { static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), static_cast<int32_t>(extent.depth) };
        //    
        //    VkImageBlit imageBlitRegion = {};
        //    imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    imageBlitRegion.srcSubresource.mipLevel = mipLevel;
        //    imageBlitRegion.srcSubresource.baseArrayLayer = arrayLayer;
        //    imageBlitRegion.srcSubresource.layerCount = 1;
        //    imageBlitRegion.srcOffsets[1] = blitSize;
        //    imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    imageBlitRegion.dstSubresource.mipLevel = 0;
        //    imageBlitRegion.dstSubresource.baseArrayLayer = 0;
        //    imageBlitRegion.dstSubresource.layerCount = 1;
        //    imageBlitRegion.dstOffsets[1] = blitSize;

        //    vkCmdBlitImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);
        //}
        //else
        //{
        //    // Otherwise use image copy (requires us to manually flip components).
        //    VkImageCopy imageCopyRegion = {};
        //    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    imageCopyRegion.srcSubresource.mipLevel = mipLevel;
        //    imageCopyRegion.srcSubresource.baseArrayLayer = arrayLayer;
        //    imageCopyRegion.srcSubresource.layerCount = 1;
        //    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //    imageCopyRegion.dstSubresource.mipLevel = 0;
        //    imageCopyRegion.dstSubresource.baseArrayLayer = 0;
        //    imageCopyRegion.dstSubresource.layerCount = 1;
        //    imageCopyRegion.extent = extent;

        //    vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
        //}

        //// Transition destination image to general layout, which is the required layout for mapping the image memory later on.
        //InsertImageMemoryBarrier(commandBuffer, dstImage, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
        //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1, 0);

        //// Transition back the image after the blit is done.
        //InsertImageMemoryBarrier(commandBuffer, srcImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImageLayout,
        //    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 1, mipLevel, 1, arrayLayer);

        //commandBuffer.SubmitIdle();

        //return supportsBlit;
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
                return texels * 1;
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
                return texels * 2;
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
                return texels * 3;
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
            case VK_FORMAT_G8B8G8R8_422_UNORM:
            case VK_FORMAT_B8G8R8G8_422_UNORM:
                return texels * 4;
            case VK_FORMAT_R16G16B16_UNORM:
            case VK_FORMAT_R16G16B16_SNORM:
            case VK_FORMAT_R16G16B16_USCALED:
            case VK_FORMAT_R16G16B16_SSCALED:
            case VK_FORMAT_R16G16B16_UINT:
            case VK_FORMAT_R16G16B16_SINT:
            case VK_FORMAT_R16G16B16_SFLOAT:
                return texels * 6;
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
            case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
            case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
            case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
            case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
            case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
            case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
            case VK_FORMAT_G16B16G16R16_422_UNORM:
            case VK_FORMAT_B16G16R16G16_422_UNORM:
                return texels * 8;
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R32G32B32_SINT:
            case VK_FORMAT_R32G32B32_SFLOAT:
                return texels * 12;
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R32G32B32A32_SINT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R64G64_UINT:
            case VK_FORMAT_R64G64_SINT:
            case VK_FORMAT_R64G64_SFLOAT:
                return texels * 16;
            case VK_FORMAT_R64G64B64_UINT:
            case VK_FORMAT_R64G64B64_SINT:
            case VK_FORMAT_R64G64B64_SFLOAT:
                return texels * 24;
            case VK_FORMAT_R64G64B64A64_UINT:
            case VK_FORMAT_R64G64B64A64_SINT:
            case VK_FORMAT_R64G64B64A64_SFLOAT:
                return texels * 32;
            // compressed texture formats
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            case VK_FORMAT_BC4_UNORM_BLOCK:
            case VK_FORMAT_BC4_SNORM_BLOCK:
                return eastl::max(extents.width / 4u, 1u) * eastl::max(extents.height / 4u, 1u) * extents.depth * 8;
            case VK_FORMAT_BC2_UNORM_BLOCK:
            case VK_FORMAT_BC2_SRGB_BLOCK:
            case VK_FORMAT_BC3_UNORM_BLOCK:
            case VK_FORMAT_BC3_SRGB_BLOCK:
            case VK_FORMAT_BC5_UNORM_BLOCK:
            case VK_FORMAT_BC5_SNORM_BLOCK:
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            case VK_FORMAT_BC7_UNORM_BLOCK:
            case VK_FORMAT_BC7_SRGB_BLOCK:
                return eastl::max(extents.width / 4u, 1u) * eastl::max(extents.height / 4u, 1u) * extents.depth * 16;
            // depth texture formats
            case VK_FORMAT_S8_UINT:
                return texels * 1;
            case VK_FORMAT_D16_UNORM:
                return texels * 2;
            case VK_FORMAT_D16_UNORM_S8_UINT:
                return texels * 3;
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
                return texels * 4;
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return texels * 5;
            default:
                Logger::ErrorTF(LOG_TAG, "Cannot compute reqired image size, unsupported format: &u", static_cast<uint32_t>(format));
                break;
        }
    }
}