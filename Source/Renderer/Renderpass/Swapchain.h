#pragma once

#include "Mantis.h"

namespace Mantis
{
    class Window;
    class Surface;

    class Swapchain
    {
    public:
        /// <summary>
        /// Creates a new swapchain.
        /// </summary>
        /// <param name="window">The window this swapchain outpts to.</param>
        /// <param name="resolution">The size of the images in the chain.</param>
        /// <param name="vsync">Should vsync be used.</param>
        /// <param name="oldSwapchain">The previous swap chain, if applicable.</param>
        explicit Swapchain(const Window* window, const Vector2Int& resolution, bool vsync, const Swapchain* oldSwapchain = nullptr);

        ~Swapchain();

        /// <summary>
        /// Acquires the next image in the swapchain into the internal acquired image. The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX.
        /// </summary>
        /// <param name="presentCompleteSemaphore">Anoptional semaphore that is signaled when the image is ready for use.</param>
        /// <returns>Result of the image acquisition.</returns>
        VkResult AcquireNextImage(const VkSemaphore& presentCompleteSemaphore = VK_NULL_HANDLE);

        /// <summary>
        /// Queue an image for presentation using the internal acquired image for queue presentation.
        /// </summary>
        /// <param name="presentQueue">Presentation queue for presenting the image.</param>
        /// <param name="waitSemaphore">An optional semaphore that is waited on before the image is presented.</param>
        /// <returns>Result of the queue presentation.</returns>
        VkResult QueuePresent(const VkQueue& presentQueue, const VkSemaphore& waitSemaphore = VK_NULL_HANDLE);

        /// <summary>
        /// Gets the underlying swapchain.
        /// </summary>
        operator const VkSwapchainKHR& () const { return m_swapchain; }

        /// <summary>
        /// Gets the underlying swapchain.
        /// </summary>
        const VkSwapchainKHR& GetSwapchain() const { return m_swapchain; }

        /// <summary>
        /// Gets the resolution of the swapchain.
        /// </summary>
        const Vector2Int& GetResolution() const
        { 
            return Vector2Int(
                static_cast<int>(m_extent.width), 
                static_cast<int>(m_extent.height));
        }

        /// <summary>
        /// Gest the number of images in the swapchain.
        /// </summary>
        /// <returns></returns>
        const uint32_t& GetImageCount() const { return m_imageCount; }

        /// <summary>
        /// Gets the transformation applied to the swapchain images on presentation.
        /// </summary>
        const VkSurfaceTransformFlagsKHR& GetPreTransform() const { return m_preTransform; }

        /// <summary>
        /// Gets the alpha composite mode used.
        /// </summary>
        const VkCompositeAlphaFlagBitsKHR& GetCompositeAlpha() const { return m_compositeAlpha; }

        /// <summary>
        /// Gets all the images in the swapchain.
        /// </summary>
        const eastl::vector<VkImage>& GetImages() const { return m_images; }

        /// <summary>
        /// Gets the currently active image in the swapchain.
        /// </summary>
        const VkImage& GetActiveImage() const { return m_images[m_activeImageIndex]; }

        /// <summary>
        /// Gets the image view for all images in the swapchain.
        /// </summary>
        const eastl::vector<VkImageView>& GetImageViews() const { return m_imageViews; }

        /// <summary>
        /// Gets the active image index.
        /// </summary>
        const uint32_t& GetActiveImageIndex() const { return m_activeImageIndex; }

    private:

        void ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Vector2Int& targetResolution);

        void ChoosePresentMode(const eastl::vector<VkPresentModeKHR>& supportedModes, bool vsync);

        bool IsPresentModeSupported(const eastl::vector<VkPresentModeKHR>& supportedModes, VkPresentModeKHR presentMode) const;

        void ChooseTransform(const VkSurfaceCapabilitiesKHR& capabilities);

        void ChooseAlphaComposite(const VkSurfaceCapabilitiesKHR& capabilities);

        void CreateSwapchain(const Surface* surface, const Swapchain* oldSwapchain);

        void CreateImageViews(const Surface* surface);

        VkSwapchainKHR m_swapchain;

        VkExtent2D m_extent;
        VkPresentModeKHR m_presentMode;
        uint32_t m_imageCount;
        VkSurfaceTransformFlagsKHR m_preTransform;
        VkCompositeAlphaFlagBitsKHR m_compositeAlpha;
        eastl::vector<VkImage> m_images;
        eastl::vector<VkImageView> m_imageViews;

        VkFence m_fenceImage;
        uint32_t m_activeImageIndex;
    };
}