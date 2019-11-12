#pragma once

#include "Mantis.h"

#include "Renderer/Utils/Nameable.h"
#include "Renderer/Descriptor/Descriptor.h"

namespace Mantis
{
	/// <summary>
	/// Additional creation options for an image.
	/// </summary>
	enum struct ImageCreateMisc
	{
		None = 0,
		GenerateMips			= 1 << 0,
		ForceArray				= 1 << 1,
		MutableSRGB				= 1 << 2,
		ConcurrentGraphics		= 1 << 3,
		ConcurrentAsyncGraphics = 1 << 4,
		ConcurrentAsyncCompute	= 1 << 5,
		ConcurrentAsyncTransfer = 1 << 6,
	};
	ENUM_IS_FLAGS(ImageCreateMisc)
	
	/// <summary>
	/// The creation options for an image.
	/// </summary>
	struct ImageCreateInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t levels = 1;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageType type = VK_IMAGE_TYPE_2D;
		uint32_t layers = 1;
		VkImageUsageFlags usage = 0;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageCreateFlags flags = 0;
		ImageCreateMisc misc = ImageCreateMisc::None;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		VkComponentMapping swizzle =
		{
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B, 
				VK_COMPONENT_SWIZZLE_A,
		};

		static ImageCreateInfo immutable_2d_image(unsigned width, unsigned height, VkFormat format, bool mipmapped = false)
		{
			ImageCreateInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.levels = mipmapped ? 0u : 1u;
			info.format = format;
			info.type = VK_IMAGE_TYPE_2D;
			info.layers = 1;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			info.misc = mipmapped ? unsigned(IMAGE_MISC_GENERATE_MIPS_BIT) : 0u;
			info.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			return info;
		}

		static ImageCreateInfo
			immutable_3d_image(unsigned width, unsigned height, unsigned depth, VkFormat format, bool mipmapped = false)
		{
			ImageCreateInfo info = immutable_2d_image(width, height, format, mipmapped);
			info.depth = depth;
			info.type = VK_IMAGE_TYPE_3D;
			return info;
		}

		static ImageCreateInfo render_target(unsigned width, unsigned height, VkFormat format)
		{
			ImageCreateInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.levels = 1;
			info.format = format;
			info.type = VK_IMAGE_TYPE_2D;
			info.layers = 1;
			info.usage = (format_has_depth_or_stencil_aspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			info.misc = 0;
			info.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
			return info;
		}

		static ImageCreateInfo transient_render_target(unsigned width, unsigned height, VkFormat format)
		{
			ImageCreateInfo info;
			info.domain = ImageDomain::Transient;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.levels = 1;
			info.format = format;
			info.type = VK_IMAGE_TYPE_2D;
			info.layers = 1;
			info.usage = (format_has_depth_or_stencil_aspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) |
				VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			info.misc = 0;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			return info;
		}
	};


    /// <summary>
    /// Manages an image.
    /// </summary>
    class Image
        : public NonCopyable
        , public Nameable
        , public Descriptor
    {
    public:
        /// <summary>
        /// Creates a new image.
        /// </summary>
        explicit Image();

        /// <summary>
        /// Destroys the image.
        /// </summary>
        virtual ~Image();

        /// <summary>
        /// Gets the underlying image instance.
        /// </summary>
        const VkImage& GetImage() const { return m_image; }

        /// <summary>
        /// The usage of this image.
        /// </summary>
        const VkImageUsageFlags& GetUsage() const { return m_usage; }

        /// <summary>
        /// The resolution of this image.
        /// </summary>
        const VkExtent3D& GetExtents() const { return m_extent; }

        /// <summary>
        /// The type of this image.
        /// </summary>
        const VkImageType& GetType() const { return m_type; }

        /// <summary>
        /// The format of this image.
        /// </summary>
        const VkFormat& GetFormat() const { return m_format; }

        /// <summary>
        /// The sample count of this image.
        /// </summary>
        const VkSampleCountFlagBits& GetSamples() const { return m_samples; }

        /// <summary>
        /// The number of mip map levels in this image.
        /// </summary>
        const uint32_t& GetLevelCount() const { return m_arrayLayers; }

        /// <summary>
        /// The number of layers in this image.
        /// </summary>
        const uint32_t& GetLayerCount() const { return m_arrayLayers; }

        /// <summary>
        /// Gets if this image can be used as a cubemap.
        /// </summary>
        const bool IsCubeCompatible() const
        {
            return HAS_FLAGS(m_flags, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) &&
                m_extent.width == m_extent.height && m_extent.depth == 1 &&
                m_arrayLayers % 6 == 0 &&
                m_samples == 1;
        }

        /// <summary>
        /// Sets the name of this instance.
        /// </summary>
        void SetName(const String& name);

        WriteDescriptorSet GetWriteDescriptor(
            const uint32_t& binding,
            const VkDescriptorType& descriptorType, 
            const eastl::optional<OffsetSize>& offsetSize
        ) const override;

        /// <summary>
        /// Gets a copy of the image contents.
        /// </summary>
        /// <param name="size">The size of the image contents in bytes.</param>
        /// <param name="extent">The resolution of the image in pixels.</param>
        /// <param name="mipLevel">The mipmap level index to sample.</param>
        /// <param name="arrayLayer">The array level to sample.</param>
        /// <returns>A copy of the image pixels.</returns>
        eastl::unique_ptr<uint8_t[]> GetContents(
            uint32_t& size,
            VkExtent3D& extent,
            const uint32_t& mipLevel = 0,
            const uint32_t& arrayLayer = 0
        ) const;

        /// <summary>
        /// Sets the contents of this image. The data must be laid out such that the
        /// mip maps for each layer are grouped in order of decending size first,
        /// then by layer.
        /// </summary>
        /// <param name="contents">The image data to set.</param>
        /// <param name="baseMipLevel">The first mip map level to copy into.</param>
        /// <param name="mipLevelCount">The number of mip map levels to set.</param>
        /// <param name="baseLayer">The first layer to copy into.</param>
        /// <param name="layerCount">The number of layers to copy.</param>
        void SetContents(
            const uint8_t* contents,
            const uint32_t& baseMipLevel = 0,
            const uint32_t& mipLevelCount = 1,
            const uint32_t& baseLayer = 0,
            const uint32_t& layerCount = 1
        );

        static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(
            const uint32_t& binding,
            const VkDescriptorType& descriptorType,
            const VkShaderStageFlags& stage,
            const uint32_t& count
        );

        /// <summary>
        /// Generates the mip maps for this image.
        /// </summary>
        /// <param name="commandBuffer">The command buffer to generate the mip maps on.</param>
        void GenerateMipmaps(const CommandBuffer& commandBuffer);

        void TransitionImageLayout(
            const CommandBuffer& commandBuffer,
            const uint32_t& srcQueueFamilyIndex,
            const uint32_t& dstQueueFamilyIndex,
            const VkImageLayout& srcImageLayout,
            const VkImageLayout& dstImageLayout
        );

        void InsertMemoryBarrier(
            const CommandBuffer& commandBuffer,
            const uint32_t& srcQueueFamilyIndex,
            const uint32_t& dstQueueFamilyIndex,
            const VkAccessFlags& srcAccessMask,
            const VkAccessFlags& dstAccessMask,
            const VkImageLayout& oldImageLayout,
            const VkImageLayout& newImageLayout,
            const VkPipelineStageFlags& srcStageMask,
            const VkPipelineStageFlags& dstStageMask
        );

        bool CopyImage(
            const VkImage& srcImage,
            VkImage& dstImage,
            VkDeviceMemory& dstImageMemory,
            const VkFormat& srcFormat,
            const VkExtent3D& extent,
            const VkImageLayout& srcImageLayout
        );

        /// <summary>
        /// Determines a suitable number of mipmap levels for an image of the given resolution.
        /// </summary>
        /// <param name="extent">The resolution of the image.</param>
        /// <returns>The number of mipmap levels.</returns>
        static MANTIS_INLINE uint32_t NumMipLevels(const VkExtent3D& extent)
        {
            uint32_t levels = 0;
            uint32_t maxDim = eastl::max(eastl::max(extent.width, extent.height), extent.depth);
            while (maxDim)
            {
                levels++;
                maxDim >>= 1;
            }
            return levels;
        }

    protected:
        /// <summary>
        /// Creates the image resources.
        /// </summary>
        /// <param name="properties">The required memory properties for the image.</param>
        /// <param name="usage">The intended usage of the image.</param>
        /// <param name="flags">The flags specified when creating the image.</param>
        /// <param name="type">The dimensionality of the image.</param>
        /// <param name="viewType">The dimensionality of the image view.</param>
        /// <param name="extent">The resolution of the image.</param>
        /// <param name="format">The format and type of the texel blocks that will be contained in the image.</param>
        /// <param name="tiling">The tiling of the image.</param>
        /// <param name="samples">The number of samples per texel.</param>
        /// <param name="mipLevels">The number of levels of detail for the image.</param>
        /// <param name="arrayLayers">The number of layers in the image.</param>
        /// <param name="filter">The sampler filtering mode.</param>
        /// <param name="addressMode">The wrapping behaviour of the sampler.</param>
        /// <param name="anisotropic">Should the sampler use anisotropic filtering.</param>
        /// <param name="compare">Should the sampler use a comparison mode.</param>
        /// <param name="compareOp">The comparison operation used by the sampler.</param>
        void Create(
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
        void CreateImage(
            const VkMemoryPropertyFlags& properties,
            const VkImageCreateFlags& flags,
            const VkImageType& type,
            const VkImageTiling& tiling
        );

        VkImage m_image;

        VmaAllocator m_allocator;
        VmaAllocation m_allocation;
        VkMemoryPropertyFlags m_memoryFlags;

        VkImageUsageFlags m_usage;
        VkImageCreateFlags m_flags;
        VkExtent3D m_extent;
        VkImageType m_type;
        VkFormat m_format;
        VkSampleCountFlagBits m_samples;
        uint32_t m_mipLevels;
        uint32_t m_arrayLayers;
    };
}