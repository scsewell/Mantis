#pragma once

#include "Mantis.h"

namespace Mantis
{
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    /// <summary>
    /// Manages a vulkan instance.
    /// </summary>
    class Instance
    {
    public:
        Instance();
        ~Instance();

        /// <summary>
        /// Gets the underlying Vulkan instance.
        /// </summary>
        operator const VkInstance& () const { return m_instance; }

        /// <summary>
        /// Gets the validation layers active on this instance.
        /// </summary>
        const eastl::vector<const char*>& GetInstanceLayers() const { return m_instanceLayers; }

        /// <summary>
        /// Gets the Vulkan extentions enabled on this instance.
        /// </summary>
        const eastl::vector<const char*>& GetInstanceExtensions() const { return m_instanceExtensions; }

        /// <summary>
        /// Gets the underlying Vulkan instance.
        /// </summary>
        const VkInstance& GetInstance() const { return m_instance; }

    private:
        static VkResult FvkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
        static void FvkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);

        /// <summary>
        /// Determines which validation layers 
        /// </summary>
        void SetupLayers();

        /// <summary>
        /// Determines which extentions to request.
        /// </summary>
        void SetupExtensions();

        /// <summary>
        /// Creates the underlying Vulkan instance.
        /// </summary>
        void CreateInstance();

        /// <summary>
        /// Hooks up the debug message callback.
        /// </summary>
        void CreateDebugCallback();

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugCallback;

        eastl::vector<const char*> m_instanceLayers;
        eastl::vector<const char*> m_instanceExtensions;
    };
}