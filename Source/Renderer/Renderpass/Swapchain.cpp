#include "stdafx.h"
#include "Swapchain.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Swapchain")

namespace Mantis
{
    /// <summary>
    /// The first supported composite mode is used for the swapchain.
    /// </summary>
    static const eastl::vector<VkCompositeAlphaFlagBitsKHR> COMPOSITE_ALPHA_FLAGS =
    {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    Swapchain::Swapchain(const Window* window, const Vector2Int& resolution, bool vsync, const Swapchain* oldSwapchain = nullptr) :
        m_swapchain(VK_NULL_HANDLE),
        m_presentMode(VK_PRESENT_MODE_FIFO_KHR),
        m_imageCount(0),
        m_preTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR),
        m_compositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR),
        m_fenceImage(VK_NULL_HANDLE),
        m_activeImageIndex(std::numeric_limits<uint32_t>::max())
    {
        auto surface = window->GetSurface();
        auto capabilities = surface->GetCapabilities();

        ChooseExtent(capabilities, resolution);
        ChoosePresentMode(surface->GetPresentationModes(), vsync);
        ChooseTransform(capabilities);
        ChooseAlphaComposite(capabilities);

        CreateSwapchain(surface, oldSwapchain);
        CreateImageViews(surface);

        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, &m_fenceImage);
    }

    Swapchain::~Swapchain()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        vkDestroySwapchainKHR(*logicalDevice, m_swapchain, nullptr);

        for (const auto& imageView : m_imageViews)
        {
            vkDestroyImageView(*logicalDevice, imageView, nullptr);
        }

        vkDestroyFence(*logicalDevice, m_fenceImage, nullptr);
    }


    void Swapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Vector2Int& targetResolution)
    {
        if (capabilities.currentExtent.width != eastl::numeric_limits<uint32_t>::max())
        {
            m_extent = capabilities.currentExtent;
        }
        else
        {
            m_extent.width = eastl::max(capabilities.minImageExtent.width, eastl::min(capabilities.maxImageExtent.width, static_cast<uint32_t>(targetResolution.x)));
            m_extent.height = eastl::max(capabilities.minImageExtent.height, eastl::min(capabilities.maxImageExtent.height, static_cast<uint32_t>(targetResolution.y)));
        }
    }

    void Swapchain::ChoosePresentMode(const eastl::vector<VkPresentModeKHR>& supportedModes, bool vsync)
    {
        if (vsync)
        {
            // try to tripple buffer if possible
            if (IsPresentModeSupported(supportedModes, VK_PRESENT_MODE_MAILBOX_KHR))
            {
                m_presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
            else if (IsPresentModeSupported(supportedModes, VK_PRESENT_MODE_FIFO_KHR))
            {
                m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
            }
            else if (IsPresentModeSupported(supportedModes, VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            else
            {
                Logger::ErrorT(LOG_TAG, "Failed to find supported presentation mode.");
            }
        }
        else
        {
            // try to present as fast as possible
            if (IsPresentModeSupported(supportedModes, VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            else if (IsPresentModeSupported(supportedModes, VK_PRESENT_MODE_FIFO_RELAXED_KHR))
            {
                m_presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            }
            else if (IsPresentModeSupported(supportedModes, VK_PRESENT_MODE_FIFO_KHR))
            {
                m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
            }
            else
            {
                Logger::ErrorT(LOG_TAG, "Failed to find supported presentation mode.");
            }
        }
    }

    bool Swapchain::IsPresentModeSupported(const eastl::vector<VkPresentModeKHR>& supportedModes, VkPresentModeKHR presentMode) const
    {
        for (const auto& mode : supportedModes)
        {
            if (mode == presentMode)
            {
                return true;
            }
        }
        return false;
    }

    void Swapchain::ChooseTransform(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        // try to not transform the image if possible
        if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            m_preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else
        {
            m_preTransform = capabilities.currentTransform;
        }
    }

    void Swapchain::ChooseAlphaComposite(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        for (const auto& compositeAlphaFlag : COMPOSITE_ALPHA_FLAGS)
        {
            if (capabilities.supportedCompositeAlpha & compositeAlphaFlag)
            {
                m_compositeAlpha = compositeAlphaFlag;
                return;
            }
        }

        Logger::ErrorT(LOG_TAG, "Failed to find a supported alpha composite mode!");
    }

    void Swapchain::CreateSwapchain(const Surface* surface, const Swapchain* oldSwapchain)
    {
        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        auto format = surface->GetFormat();
        auto capabilities = surface->GetCapabilities();

        // pick the number of images in the swap chain
        uint32_t desiredImageCount = capabilities.minImageCount + 1;

        if (capabilities.maxImageCount > 0 && desiredImageCount > capabilities.maxImageCount)
        {
            desiredImageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = *surface;
        createInfo.minImageCount = desiredImageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = m_extent;
        createInfo.imageArrayLayers = 1;
        createInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(m_preTransform);
        createInfo.compositeAlpha = m_compositeAlpha;
        createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.presentMode = m_presentMode;
        createInfo.clipped = VK_TRUE;

        // provide the old swapchain if applicable
        if (oldSwapchain != nullptr)
        {
            createInfo.oldSwapchain = *oldSwapchain;
        }
        else
        {
            createInfo.oldSwapchain = VK_NULL_HANDLE;
        }

        // determine if image sharing is required between multiple queues
        auto graphicsFamily = logicalDevice->GetGraphicsFamily();
        auto presentFamily = logicalDevice->GetPresentFamily();

        if (graphicsFamily != presentFamily)
        {
            eastl::array<uint32_t, 2> queueFamily = { graphicsFamily, presentFamily };
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamily.size());
            createInfo.pQueueFamilyIndices = queueFamily.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        // create the swapchain
        if (Renderer::Check(vkCreateSwapchainKHR(*logicalDevice, &createInfo, nullptr, &m_swapchain) ))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create swapchain!");
        }

        // get the images
        if (Renderer::Check(vkGetSwapchainImagesKHR(*logicalDevice, m_swapchain, &m_imageCount, nullptr)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to get swapchain image count!");
        }

        m_images.resize(m_imageCount);

        if (Renderer::Check(vkGetSwapchainImagesKHR(*logicalDevice, m_swapchain, &m_imageCount, m_images.data())))
        {
            Logger::ErrorT(LOG_TAG, "Failed to get swapchain images!");
        }
    }

    void Swapchain::CreateImageViews(const Surface* surface)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        m_imageViews.resize(m_imageCount);

        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = surface->GetFormat().format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (Renderer::Check(vkCreateImageView(*logicalDevice, &createInfo, nullptr, &m_imageViews[i])))
            {
                Logger::ErrorT(LOG_TAG, "Failed to get create image view!");
            }
        }
    }

    VkResult Swapchain::AcquireNextImage(const VkSemaphore& presentCompleteSemaphore)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkResult result = vkAcquireNextImageKHR(*logicalDevice, m_swapchain, std::numeric_limits<uint64_t>::max(), presentCompleteSemaphore, VK_NULL_HANDLE, &m_activeImageIndex);

        switch (result)
        {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                break;
            default:
                Renderer::Check(result);
                Logger::ErrorT(LOG_TAG, "Failed to acquire swapchain image!");
                break;
        }

        //Renderer::CheckVk(vkWaitForFences(*logicalDevice, 1, &m_fenceImage, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        //Renderer::CheckVk(vkResetFences(*logicalDevice, 1, &m_fenceImage));

        return result;
    }

    VkResult Swapchain::QueuePresent(const VkQueue& presentQueue, const VkSemaphore& waitSemaphore)
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &m_activeImageIndex;

        return vkQueuePresentKHR(presentQueue, &presentInfo);
    }
}