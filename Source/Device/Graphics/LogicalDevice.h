#pragma once

#include "Mantis.h"
#include "Instance.h"
#include "PhysicalDevice.h"

namespace Mantis
{
    class Surface;

    /// <summary>
    /// A graphics queue type.
    /// </summary>
    enum struct QueueType
    {
        Graphics,
        Present,
        Compute,
        Transfer,
    };

    /// <summary>
    /// Represents a device that can execute rendering commands.
    /// </summary>
    class LogicalDevice
    {
    public:
        explicit LogicalDevice(const Instance* instance, const PhysicalDevice* physicalDevice, const Surface* surface);

        ~LogicalDevice();

        /// <summary>
        /// Gets the underlying logical device.
        /// </summary>
        /// <returns></returns>
        operator const VkDevice& () const { return m_logicalDevice; }

        /// <summary>
        /// Gets the underlying logical device.
        /// </summary>
        const VkDevice& GetLogicalDevice() const { return m_logicalDevice; }

        /// <summary>
        /// Gets the device features that are currently enabled on this device.
        /// </summary>
        const VkPhysicalDeviceFeatures& GetEnabledFeatures() const { return m_enabledFeatures; }

        /// <summary>
        /// Gets the graphcis queue for this device.
        /// </summary>
        const VkQueue& GetGraphicsQueue() const { return m_graphicsQueue; }

        /// <summary>
        /// Gets the presentation queue for this device.
        /// </summary>
        const VkQueue& GetPresentQueue() const { return m_presentQueue; }

        /// <summary>
        /// Gets the compute queue for this device.
        /// </summary>
        const VkQueue& GetComputeQueue() const { return m_computeQueue; }

        /// <summary>
        /// Gets the transfer queue for this device.
        /// </summary>
        const VkQueue& GetTransferQueue() const { return m_transferQueue; }

        /// <summary>
        /// Gets a queue for this device.
        /// </summary>
        /// <param name="queueType">The queue type to get.</param>
        /// <returns>The queue or null on error.</returns>
        const VkQueue& GetQueue(const QueueType& queueType) const;

        /// <summary>
        /// Gets the graphcis queue family for this device.
        /// </summary>
        const uint32_t& GetGraphicsFamily() const { return m_graphicsFamily; }

        /// <summary>
        /// Gets the present queue family for this device.
        /// </summary>
        const uint32_t& GetPresentFamily() const { return m_presentFamily; }

        /// <summary>
        /// Gets the compute queue family for this device.
        /// </summary>
        const uint32_t& GetComputeFamily() const { return m_computeFamily; }

        /// <summary>
        /// Gets the tansfer queue family for this device.
        /// </summary>
        const uint32_t& GetTransferFamily() const { return m_transferFamily; }

        /// <summary>
        /// Gets a queue family index for this device.
        /// </summary>
        /// <param name="queueType">The queue type to get the family index for.</param>
        /// <returns>The queue index or VK_NULL_HANDLE on error.</returns>
        const uint32_t& LogicalDevice::GetQueueFamilyIndex(const QueueType& queueType) const;

    private:
        void CreateQueueIndices(const Surface* surface);
        void CreateLogicalDevice();
        
        /// <summary>
        /// Selects which featues we want to enable for this device.
        /// </summary>
        /// <param name="deviceFeatures">The features which are supported by the device.</param>
        /// <returns>The featues we want to enable.</returns>
        static VkPhysicalDeviceFeatures GetFeaturesToRequest(const VkPhysicalDeviceFeatures& deviceFeatures);

        const Instance* m_instance;
        const PhysicalDevice* m_physicalDevice;

        VkDevice m_logicalDevice;
        VkPhysicalDeviceFeatures m_enabledFeatures;

        VkQueueFlags m_supportedQueues;
        uint32_t m_graphicsFamily;
        uint32_t m_presentFamily;
        uint32_t m_computeFamily;
        uint32_t m_transferFamily;

        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;
    };
}
