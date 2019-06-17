#pragma once

#include "Mantis.h"

#include "Renderer/Commands/CommandBuffer.h"
#include "Renderer/Descriptor/Descriptor.h"

namespace Mantis
{
    /// <summary>
    /// Manages an image, view, and sampler.
    /// </summary>
    class Image :
        public Descriptor
    {
    public:
        /// <summary>
        /// Creates a new image.
        /// </summary>
        /// <param name="extent">The number of data elements in each dimension of the base level.</param>
        /// <param name="imageType">The dimensionality of the image.</param>
        /// <param name="format">The format and type of the texel blocks that will be contained in the image.</param>
        /// <param name="samples">The number of samples per texel.</param>
        /// <param name="tiling">The tiling arrangement of the texel blocks in memory.</param>
        /// <param name="mipLevels">The number of levels of detail of the image.</param>
        /// <param name="arrayLayers">The number of layers in the image.</param>
        /// <param name="usage">The intended usage of the image.</param>
        /// <param name="properties">The required memory properties for the image.</param>
        Image(
            const VkExtent3D& extent,
            const VkImageType& imageType,
            const VkFormat& format,
            const VkSampleCountFlagBits& samples,
            const VkImageTiling& tiling,
            const uint32_t& mipLevels,
            const uint32_t& arrayLayers,
            const VkImageUsageFlags& usage,
            const VkMemoryPropertyFlags& properties
        );

        /// <summary>
        /// Destroys the image.
        /// </summary>
        ~Image();

        WriteDescriptorSet GetWriteDescriptor(
            const uint32_t& binding,
            const VkDescriptorType& descriptorType, 
            const eastl::optional<OffsetSize>& offsetSize
        ) const override;

        /// <summary>
        /// Gets a copy of the image contents.
        /// </summary>
        /// <param name="extent">The resolution of the image in pixels.</param>
        /// <param name="mipLevel">The mipmap level index to sample.</param>
        /// <param name="arrayLayer">The array level to sample.</param>
        /// <returns>A copy of the image pixels.</returns>
        eastl::unique_ptr<uint8_t[]> GetContents(
            VkExtent3D& extent,
            const uint32_t& mipLevel = 0,
            const uint32_t& arrayLayer = 0
        ) const;

        /// <summary>
        /// Sets the contents of this image.
        /// </summary>
        /// <param name="contents">The image data to set.</param>
        /// <param name="layerCount">The amount of layers contained in the contents.</param>
        /// <param name="baseArrayLayer">The first layer to copy into.</param>
        void SetContents(const uint8_t* contents, const uint32_t& layerCount, const uint32_t& baseArrayLayer);

        /// <summary>
        /// Gets the underlying image instance.
        /// </summary>
        const VkImage& GetImage() { return m_image; }

        /// <summary>
        /// Gets the image view for this image.
        /// </summary>
        const VkImageView& GetView() const { return m_view; }

        const VkExtent3D& GetExtent() const { return m_extent; }

        const VkFormat& GetFormat() const { return m_format; }

        const VkSampleCountFlagBits& GetSamples() const { return m_samples; }

        const VkImageUsageFlags& GetUsage() const { return m_usage; }

        const uint32_t& GetMipLevels() const { return m_mipLevels; }

        const uint32_t& GetArrayLevels() const { return m_arrayLayers; }

        /// <summary>
        /// Gets the sampler for this image.
        /// </summary>
        const VkSampler& GetSampler() const { return m_sampler; }

        //const VkImageLayout& GetLayout() const { return m_layout; }

        //const VkFilter& GetFilter() const { return m_filter; }

        //const VkSamplerAddressMode& GetAddressMode() const { return m_addressMode; }

        //const bool& IsAnisotropic() const { return m_anisotropic; }

        static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(
            const uint32_t& binding,
            const VkDescriptorType& descriptorType,
            const VkShaderStageFlags& stage,
            const uint32_t& count
        );

        /// <summary>
        /// Determines a suitable number of mipmap levels for an image of the given resolution.
        /// </summary>
        /// <param name="extent">The resolution of the image.</param>
        /// <returns>The number of mipmap levels.</returns>
        static uint32_t GetMipLevels(const VkExtent3D& extent);

        /// <summary>
        /// Checks if a format has a depth component
        /// </summary>
        /// <param name="format">The format to check.</param>
        static bool HasDepth(const VkFormat& format);

        /// <summary>
        /// Checks if a format has a stencil component.
        /// </summary>
        /// <param name="format">The format to check.</param>
        static bool HasStencil(const VkFormat& format);

        static void CreateMipmaps(
            const VkImage& image,
            const VkExtent3D& extent,
            const VkFormat& format,
            const VkImageLayout& dstImageLayout,
            const uint32_t& mipLevels,
            const uint32_t& baseArrayLayer,
            const uint32_t& layerCount
        );

        static void TransitionImageLayout(
            const VkImage& image,
            const VkFormat& format,
            const VkImageLayout& srcImageLayout,
            const VkImageLayout& dstImageLayout,
            const VkImageAspectFlags& imageAspect,
            const uint32_t& mipLevels,
            const uint32_t& baseMipLevel,
            const uint32_t& layerCount,
            const uint32_t& baseArrayLayer
        );

        static void InsertImageMemoryBarrier(
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
        );

        static void CopyBufferToImage(
            const VkBuffer& buffer,
            const VkImage& image,
            const VkExtent3D& extent,
            const uint32_t& layerCount,
            const uint32_t& baseArrayLayer
        );

        static bool CopyImage(
            const VkImage& srcImage,
            VkImage& dstImage,
            VkDeviceMemory& dstImageMemory,
            const VkFormat& srcFormat,
            const VkExtent3D& extent,
            const VkImageLayout& srcImageLayout,
            const uint32_t& mipLevel,
            const uint32_t& arrayLayer
        );

    protected:
        static void CreateImage(
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
        );

        static void CreateImageView(
            VkImageView& imageView,
            const VkImage& image,
            const VkImageViewType& type,
            const VkFormat& format,
            const VkImageAspectFlags& imageAspect,
            const uint32_t& mipLevels,
            const uint32_t& baseMipLevel,
            const uint32_t& layerCount,
            const uint32_t& baseArrayLayer
        );

        static void CreateImageSampler(
            VkSampler& sampler,
            const VkFilter& filter,
            const VkSamplerAddressMode& addressMode,
            const bool& anisotropic,
            const uint32_t& mipLevels,
            const bool& compare = false,
            const VkCompareOp& compareOp = VK_COMPARE_OP_ALWAYS
        );

        /// <summary>
        /// Gets the number of bytes required to fit the specified texture.
        /// </summary>
        /// <param name="extents">The extents of the image.</param>
        /// <param name="format">The format of the image.</param>
        /// <returns>The size in bytes.</returns>
        static VkDeviceSize GetSize(const VkExtent3D& extents, const VkFormat& format);

    private:
        VkImage m_image;
        VkImageView m_view;
        VkSampler m_sampler;

        VmaAllocator m_allocator;
        VmaAllocation m_allocation;
        VkMemoryPropertyFlags m_memoryFlags;

        VkExtent3D m_extent;
        VkFormat m_format;
        VkSampleCountFlagBits m_samples;
        VkImageTiling m_tiling;
        VkImageUsageFlags m_usage;
        uint32_t m_mipLevels;
        uint32_t m_arrayLayers;

        //VkFilter m_filter;
        //VkSamplerAddressMode m_addressMode;
        //bool m_anisotropic;

        //VkImageLayout m_layout;
    };
}