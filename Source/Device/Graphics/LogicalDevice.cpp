#include "stdafx.h"
#include "LogicalDevice.h"

#include "Renderer/Renderer.h"
#include "Surface.h"

#define LOG_TAG MANTIS_TEXT("LogicalDevice")

namespace Mantis
{
    LogicalDevice::LogicalDevice(const Instance* instance, const PhysicalDevice* physicalDevice, const Surface* surface) :
        m_instance(instance),
        m_physicalDevice(physicalDevice),
        m_logicalDevice(VK_NULL_HANDLE),
        m_supportedQueues(0),
        m_graphicsFamily(0),
        m_presentFamily(0),
        m_computeFamily(0),
        m_transferFamily(0),
        m_graphicsQueue(VK_NULL_HANDLE),
        m_presentQueue(VK_NULL_HANDLE),
        m_computeQueue(VK_NULL_HANDLE),
        m_transferQueue(VK_NULL_HANDLE)
    {
        CreateQueueIndices(surface);
        CreateLogicalDevice();
    }

    LogicalDevice::~LogicalDevice()
    {
        if (Renderer::Check(vkDeviceWaitIdle(m_logicalDevice)))
        {
            Logger::ErrorT(LOG_TAG, "Wait for device to idle failed when destoying device!");
        }

        vkDestroyDevice(m_logicalDevice, nullptr);
    }

    void LogicalDevice::CreateQueueIndices(const Surface* surface)
    {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(*m_physicalDevice, &queueFamilyCount, nullptr);

        eastl::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(*m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        int32_t graphicsFamily = -1;
        int32_t presentFamily = -1;
        int32_t computeFamily = -1;
        int32_t transferFamily = -1;

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            // check for graphics support
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsFamily = i;
                m_graphicsFamily = i;
                m_supportedQueues |= VK_QUEUE_GRAPHICS_BIT;
            }

            // check for presentation support
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(*m_physicalDevice, i, *surface, &presentSupport);

            if (queueFamilies[i].queueCount > 0 && presentSupport)
            {
                presentFamily = i;
                m_presentFamily = i;
            }

            // check for compute support
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                computeFamily = i;
                m_computeFamily = i;
                m_supportedQueues |= VK_QUEUE_COMPUTE_BIT;
            }

            // check for transfer support
            if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                transferFamily = i;
                m_transferFamily = i;
                m_supportedQueues |= VK_QUEUE_TRANSFER_BIT;
            }

            if (graphicsFamily != -1 && presentFamily != -1 && computeFamily != -1 && transferFamily != -1)
            {
                break;
            }
        }

        if (graphicsFamily == -1)
        {
            Logger::ErrorT(LOG_TAG, "Failed to find queue family supporting VK_QUEUE_GRAPHICS_BIT!");
        }
    }

    void LogicalDevice::CreateLogicalDevice()
    {
        eastl::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriorities[] = { 0.0f };

        if (m_supportedQueues & VK_QUEUE_GRAPHICS_BIT)
        {
            VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
            graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphicsQueueCreateInfo.queueFamilyIndex = m_graphicsFamily;
            graphicsQueueCreateInfo.queueCount = 1;
            graphicsQueueCreateInfo.pQueuePriorities = queuePriorities;
            queueCreateInfos.emplace_back(graphicsQueueCreateInfo);
        }
        else
        {
            m_graphicsFamily = VK_NULL_HANDLE;
        }

        if (m_supportedQueues & VK_QUEUE_COMPUTE_BIT && m_computeFamily != m_graphicsFamily)
        {
            VkDeviceQueueCreateInfo computeQueueCreateInfo = {};
            computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            computeQueueCreateInfo.queueFamilyIndex = m_computeFamily;
            computeQueueCreateInfo.queueCount = 1;
            computeQueueCreateInfo.pQueuePriorities = queuePriorities;
            queueCreateInfos.emplace_back(computeQueueCreateInfo);
        }
        else
        {
            m_computeFamily = m_graphicsFamily;
        }

        if (m_supportedQueues & VK_QUEUE_TRANSFER_BIT && m_transferFamily != m_graphicsFamily && m_transferFamily != m_computeFamily)
        {
            VkDeviceQueueCreateInfo transferQueueCreateInfo = {};
            transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            transferQueueCreateInfo.queueFamilyIndex = m_transferFamily;
            transferQueueCreateInfo.queueCount = 1;
            transferQueueCreateInfo.pQueuePriorities = queuePriorities;
            queueCreateInfos.emplace_back(transferQueueCreateInfo);
        }
        else
        {
            m_transferFamily = m_graphicsFamily;
        }

        m_enabledFeatures = GetFeaturesToRequest(m_physicalDevice->GetFeatures());

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_instance->GetInstanceLayers().size());
        deviceCreateInfo.ppEnabledLayerNames = m_instance->GetInstanceLayers().data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_physicalDevice->GetExtentions().size());
        deviceCreateInfo.ppEnabledExtensionNames = m_physicalDevice->GetExtentions().data();
        deviceCreateInfo.pEnabledFeatures = &m_enabledFeatures;

        if (Renderer::Check(vkCreateDevice(*m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create logical device!");
        }

        vkGetDeviceQueue(m_logicalDevice, m_graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, m_presentFamily, 0, &m_presentQueue);
        vkGetDeviceQueue(m_logicalDevice, m_computeFamily, 0, &m_computeQueue);
        vkGetDeviceQueue(m_logicalDevice, m_transferFamily, 0, &m_transferQueue);
    }

    VkPhysicalDeviceFeatures LogicalDevice::GetFeaturesToRequest(const VkPhysicalDeviceFeatures& deviceFeatures)
    {
        VkPhysicalDeviceFeatures enabledFeatures = {};

        // enable sample rate shading filtering if supported
        if (deviceFeatures.sampleRateShading)
        {
            enabledFeatures.sampleRateShading = VK_TRUE;
        }

        // fill mode non solid is required for wireframe display
        if (deviceFeatures.fillModeNonSolid)
        {
            enabledFeatures.fillModeNonSolid = VK_TRUE;

            // wide lines must be present for line width > 1.0f
            if (deviceFeatures.wideLines)
            {
                enabledFeatures.wideLines = VK_TRUE;
            }
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support wireframe pipelines!");
        }

        if (deviceFeatures.samplerAnisotropy)
        {
            enabledFeatures.samplerAnisotropy = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support sampler anisotropy!");
        }

        if (deviceFeatures.textureCompressionBC)
        {
            enabledFeatures.textureCompressionBC = VK_TRUE;
        }
        else if (deviceFeatures.textureCompressionASTC_LDR)
        {
            enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
        }
        else if (deviceFeatures.textureCompressionETC2)
        {
            enabledFeatures.textureCompressionETC2 = VK_TRUE;
        }

        if (deviceFeatures.vertexPipelineStoresAndAtomics)
        {
            enabledFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support vertex pipeline stores and atomics!");
        }

        if (deviceFeatures.fragmentStoresAndAtomics)
        {
            enabledFeatures.fragmentStoresAndAtomics = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support fragment stores and atomics!");
        }

        if (deviceFeatures.shaderStorageImageExtendedFormats)
        {
            enabledFeatures.shaderStorageImageExtendedFormats = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support shader storage extended formats!");
        }

        if (deviceFeatures.shaderStorageImageWriteWithoutFormat)
        {
            enabledFeatures.shaderStorageImageWriteWithoutFormat = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support shader storage write without format!");
        }

        if (deviceFeatures.geometryShader)
        {
            enabledFeatures.geometryShader = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support geometry shaders!");
        }

        if (deviceFeatures.tessellationShader)
        {
            enabledFeatures.tessellationShader = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support tessellation shaders!");
        }

        if (deviceFeatures.multiViewport)
        {
            enabledFeatures.multiViewport = VK_TRUE;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Selected GPU does not support multi viewports!");
        }

        return enabledFeatures;
    }

    const VkQueue& LogicalDevice::GetQueue(const QueueType& queueType) const
    {
        switch (queueType)
        {
            case QueueType::Graphics:   return m_graphicsQueue;
            case QueueType::Present:    return m_presentQueue;
            case QueueType::Compute:    return m_computeQueue;
            case QueueType::Transfer:   return m_transferQueue;
            default:
                Logger::ErrorT(LOG_TAG, "Unsupported queue type!");
                return nullptr;
        }
    }

    const uint32_t& LogicalDevice::GetQueueFamilyIndex(const QueueType& queueType) const
    {
        switch (queueType)
        {
            case QueueType::Graphics:   return m_graphicsFamily;
            case QueueType::Present:    return m_presentFamily;
            case QueueType::Compute:    return m_computeFamily;
            case QueueType::Transfer:   return m_transferFamily;
            default:
                Logger::ErrorT(LOG_TAG, "Unsupported queue type!");
                return VK_NULL_HANDLE;
        }
    }
}
