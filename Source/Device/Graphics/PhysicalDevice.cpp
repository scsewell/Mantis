#include "stdafx.h"
#include "PhysicalDevice.h"

#define LOG_TAG MANTIS_TEXT("PhysicalDevice")

namespace Mantis
{
    static const eastl::vector<const char*> REQUIRED_DEVICE_EXTENTIONS =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    static const eastl::vector<const char*> OPTIONAL_DEVICE_EXTENTIONS =
    {
    };

    static const eastl::vector<VkSampleCountFlagBits> SAMPLE_FLAG_BITS = 
    { 
        VK_SAMPLE_COUNT_64_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_2_BIT,
    };

    PhysicalDevice::PhysicalDevice(const Instance* instance) :
        m_instance(instance),
        m_physicalDevice(VK_NULL_HANDLE),
        m_properties({}),
        m_memoryProperties({}),
        m_features({}),
        m_msaaSamples(VK_SAMPLE_COUNT_1_BIT),
        m_extentions({})
    {
        // get all GPUs
        uint32_t physicalDeviceCount;
        vkEnumeratePhysicalDevices(*m_instance, &physicalDeviceCount, nullptr);

        eastl::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(*m_instance, &physicalDeviceCount, physicalDevices.data());

        // select the best GPU
        m_physicalDevice = ChoosePhysicalDevice(physicalDevices);

        if (m_physicalDevice == nullptr)
        {
            Logger::ErrorT(LOG_TAG, "Failed to find a suitable GPU");
        }

        // get the device's capabilities
        vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);
        vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
        m_msaaSamples = GetMaxUsableSampleCount(m_properties);

        // get the extentions to request on this device
        auto supportedExtentions = GetSupportedExtentions(m_physicalDevice);
        for (const auto& extention : GetExtentions(supportedExtentions, REQUIRED_DEVICE_EXTENTIONS))
        {
            m_extentions.push_back(extention);
        }
        for (const auto& extention : GetExtentions(supportedExtentions, OPTIONAL_DEVICE_EXTENTIONS))
        {
            m_extentions.push_back(extention);
        }

        Logger::InfoTF(LOG_TAG, "Selected device: %s ID: %i ", m_properties.deviceName, m_properties.deviceID);
    }

    VkPhysicalDevice PhysicalDevice::ChoosePhysicalDevice(const eastl::vector<VkPhysicalDevice>& devices)
    {
        // Sort all the devices by rank
        eastl::vector_multimap<int32_t, VkPhysicalDevice> rankedDevices;

        for (const auto& device : devices)
        {
            int32_t score = ScorePhysicalDevice(device);
            rankedDevices.emplace(score, device);
        }

        // make sure the best candidate scored higher than 0
        if (rankedDevices.rbegin()->first > 0)
        {
            return rankedDevices.rbegin()->second;
        }

        return nullptr;
    }

    int32_t PhysicalDevice::ScorePhysicalDevice(const VkPhysicalDevice& device)
    {
        // get device information
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);

        eastl::vector<VkExtensionProperties> supportedExtentions = GetSupportedExtentions(device);

        LogDeviceInfo(physicalDeviceProperties, supportedExtentions);

        // require important extensions to be supported
        if (GetExtentions(supportedExtentions, REQUIRED_DEVICE_EXTENTIONS).size() != REQUIRED_DEVICE_EXTENTIONS.size())
        {
            return 0;
        }

        // rank the device by its capabilities
        int32_t score = 0;

        // check for optional extention support
        score += static_cast<int32_t>(GetExtentions(supportedExtentions, OPTIONAL_DEVICE_EXTENTIONS).size()) * 1000;

        // adds a large score boost for discrete GPUs
        if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 10000;
        }

        // gives a higher score to devices with a higher maximum texture size
        score += physicalDeviceProperties.limits.maxImageDimension2D;

        return score;
    }

    eastl::vector<VkExtensionProperties> PhysicalDevice::GetSupportedExtentions(const VkPhysicalDevice& device)
    {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

        eastl::vector<VkExtensionProperties> extensionProperties(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensionProperties.data());

        return extensionProperties;
    }

    eastl::vector<const char*> PhysicalDevice::GetExtentions(const eastl::vector<VkExtensionProperties>& supportedExtentions, const eastl::vector<const char*>& toGet)
    {
        eastl::vector<const char*> extentions;

        for (const auto& extentionName : toGet)
        {
            for (const auto& extension : supportedExtentions)
            {
                if (strcmp(extentionName, extension.extensionName) == 0)
                {
                    extentions.push_back(extentionName);
                    break;
                }
            }
        }

        return extentions;
    }

    VkSampleCountFlagBits PhysicalDevice::GetMaxUsableSampleCount(const VkPhysicalDeviceProperties& deviceProperties)
    {
        VkSampleCountFlags counts = eastl::min(deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts);

        for (const auto& sampleFlag : SAMPLE_FLAG_BITS)
        {
            if (counts & sampleFlag)
            {
                return sampleFlag;
            }
        }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkMemoryPropertyFlags PhysicalDevice::GetMemoryPropertyFlags(const uint32_t& memoryTypeBit) const
    {
        for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
        {
            uint32_t memoryType = 1 << i;
            if (memoryType == memoryTypeBit)
            {
                return m_memoryProperties.memoryTypes[memoryType].propertyFlags;
            }
        }

        Logger::InfoTF(LOG_TAG, "Unable to find memory type!");
    }

    VkFormat PhysicalDevice::FindSupportedFormat(const eastl::vector<VkFormat>& candidates, const VkImageTiling& tiling, const VkFormatFeatureFlags& features) const
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        return VK_FORMAT_UNDEFINED;
    }

    void PhysicalDevice::LogDeviceInfo(const VkPhysicalDeviceProperties& physicalDeviceProperties, const eastl::vector<VkExtensionProperties>& extensionProperties)
    {
        String device = String();

        switch (physicalDeviceProperties.vendorID)
        {
            case 0x8086:
                device.append("Intel");
                break;
            case 0x10DE:
                device.append("Nvidia");
                break;
            case 0x1002:
                device.append("AMD");
                break;
            default:
                device.append_sprintf("%u", physicalDeviceProperties.vendorID);
                break;
        }

        device.append_sprintf(" %s", physicalDeviceProperties.deviceName);

        switch (static_cast<int>(physicalDeviceProperties.deviceType))
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                device.append(" (Integrated)");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                device.append(" (Discrete)");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                device.append(" (Virtual)");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                device.append(" (CPU)");
                break;
            default:
                device.append(" (Other)");
                break;
        }

        device.append_sprintf(" ID: %u", physicalDeviceProperties.deviceID);

        device.append_sprintf(" Vulkan: %u.%u.%u", 
            VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
            VK_VERSION_MINOR(physicalDeviceProperties.apiVersion),
            VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));

        device.append(" Extensions:");
        for (const auto& extension : extensionProperties)
        {
            device.append_sprintf(" %s", extension.extensionName);
        }

        Logger::InfoT(LOG_TAG, device);
    }
}
