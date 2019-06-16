#pragma once

#include "Mantis.h"
#include "Instance.h"

namespace Mantis
{
    /// <summary>
    /// Represents a physical GPU.
    /// </summary>
    class PhysicalDevice
    {
    public:
        explicit PhysicalDevice(const Instance* instance);

        /// <summary>
        /// Gets the underlying physical device.
        /// </summary>
        operator const VkPhysicalDevice& () const { return m_physicalDevice; }

        /// <summary>
        /// Gets the underlying physical device.
        /// </summary>
        const VkPhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }

        /// <summary>
        /// Gets the device properties.
        /// </summary>
        const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }

        /// <summary>
        /// Gets the memory properties of this device.
        /// </summary>
        const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_memoryProperties; }

        /// <summary>
        /// Gets the features supported by this device.
        /// </summary>
        const VkPhysicalDeviceFeatures& GetFeatures() const { return m_features; }

        /// <summary>
        /// Gets the number of MSAA samples supported by this device.
        /// </summary>
        const VkSampleCountFlagBits& GetMsaaSamples() const { return m_msaaSamples; }

        /// <summary>
        /// Gets the extentions to use on this device.
        /// </summary>
        const eastl::vector<const char*>& GetExtentions() const { return m_extentions; }

        /// <summary>
        /// Gets the memory property flags for a memory type.
        /// </summary>
        /// <param name="memoryTypeBit">The type of memory.</param>
        VkMemoryPropertyFlags PhysicalDevice::GetMemoryPropertyFlags(const uint32_t& memoryTypeBit) const;

    private:
        /// <summary>
        /// Determines the most suitable device from a list of devices.
        /// </summary>
        /// <param name="devices">The devices to rank.</param>
        /// <returns>The best ranking device.</returns>
        static VkPhysicalDevice ChoosePhysicalDevice(const eastl::vector<VkPhysicalDevice>& devices);

        /// <summary>
        /// Ranks the capabilities of a device.
        /// </summary>
        /// <param name="device">The device to rank.</param>
        /// <returns>The score of the device. If zero or less the device does not support required features.</returns>
        static int32_t ScorePhysicalDevice(const VkPhysicalDevice& device);

        /// <summary>
        /// Gets all extenstions supported by a device.
        /// </summary>
        /// <param name="device">The device to get the extentions for.</param>
        /// <returns>A new vector containing the supported extentions</returns>
        static eastl::vector<VkExtensionProperties> GetSupportedExtentions(const VkPhysicalDevice& device);

        /// <summary>
        /// Gets the provided extentions that are supported.
        /// </summary>
        /// <param name="supportedExtentions">The extentions supported by a device.</param>
        /// <returns>The supported extentions.</returns>
        static eastl::vector<const char*> PhysicalDevice::GetExtentions(const eastl::vector<VkExtensionProperties>& supportedExtentions, const eastl::vector<const char*>& toGet);

        /// <summary>
        /// Determines the maximum number of MSAA samples the device can support.
        /// </summary>
        /// <param name="deviceProperties">The device capabilities.</param>
        static VkSampleCountFlagBits GetMaxUsableSampleCount(const VkPhysicalDeviceProperties& deviceProperties);

        /// <summary>
        /// Logs information about a device.
        /// </summary>
        /// <param name="physicalDeviceProperties">The device properties.</param>
        /// <param name="extensionProperties">The extentions supported by the device.</param>
        static void LogDeviceInfo(const VkPhysicalDeviceProperties& deviceProperties, const eastl::vector<VkExtensionProperties>& extensionProperties);

        const Instance* m_instance;
        VkPhysicalDevice m_physicalDevice;

        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceMemoryProperties m_memoryProperties;
        VkPhysicalDeviceFeatures m_features;
        VkSampleCountFlagBits m_msaaSamples;
        eastl::vector<const char*> m_extentions;
    };
}