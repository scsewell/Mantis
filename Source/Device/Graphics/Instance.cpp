#include "stdafx.h"
#include "Instance.h"

#include "Renderer/Renderer.h"
#include "Device/Window/Window.h"

#define LOG_TAG MANTIS_TEXT("Instance")

namespace Mantis
{
    static const eastl::vector<const char*> VALIDATION_LAYERS =
    {
        "VK_LAYER_KHRONOS_validation",
    };

    static const eastl::vector<const char*> INSTANCE_EXTENTIONS =
    {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            Logger::InfoT("Vulkan", pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            Logger::WarningT("Vulkan", pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            Logger::ErrorT("Vulkan", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    VkResult Instance::FvkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        }

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void Instance::FvkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (func != nullptr)
        {
            func(instance, messenger, pAllocator);
        }
    }

    Instance::Instance() :
        m_debugCallback(VK_NULL_HANDLE),
        m_instance(VK_NULL_HANDLE)
    {
        SetupLayers();
        SetupExtensions();
        CreateInstance();
        CreateDebugCallback();
    }

    Instance::~Instance()
    {
        FvkDestroyDebugUtilsMessengerEXT(m_instance, m_debugCallback, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    void Instance::SetupLayers()
    {
        uint32_t propertyCount;
        vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);

        eastl::vector<VkLayerProperties> layerProperties(propertyCount);
        vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data());

        String layers = "Available layers: ";
        for (const auto& layer : layerProperties)
        {
            layers.append_sprintf("%s ", layer);
        }
        Logger::InfoT(LOG_TAG, layers);

        // Sets up the layers
#if defined(MANTIS_DEBUG)
        for (const auto& layerName : VALIDATION_LAYERS)
        {
            bool layerFound = false;

            for (const auto& layerProperties : layerProperties)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                Logger::WarningTF(LOG_TAG, "Validation layer not found: '%s'", layerName);
                continue;
            }

            m_instanceLayers.emplace_back(layerName);
        }
#endif
    }

    void Instance::SetupExtensions()
    {
        auto instanceExtensions = Window::GetInstanceExtensions();

        for (uint32_t i = 0; i < instanceExtensions.second; i++)
        {
            m_instanceExtensions.emplace_back(instanceExtensions.first[i]);
        }

        for (const auto& instanceExtension : INSTANCE_EXTENTIONS)
        {
            m_instanceExtensions.emplace_back(instanceExtension);
        }

#if defined(MANTIS_DEBUG)
        m_instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    }

    void Instance::CreateInstance()
    {
        VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = MANTIS_PROJECT_NAME;
        applicationInfo.applicationVersion = VK_MAKE_VERSION(MANTIS_VERSION_MAJOR, MANTIS_VERSION_MINOR, MANTIS_VERSION_PATCH);
        applicationInfo.pEngineName = "Mantis Engine";
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_instanceLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensions.data();

        String layers = "Requested layers: ";
        for (const auto& layer : m_instanceLayers)
        {
            layers.append_sprintf("%s ", layer);
        }
        Logger::InfoT(LOG_TAG, layers);

        String extentions = "Requested instance extentions: ";
        for (const auto& extension : m_instanceExtensions)
        {
            extentions.append_sprintf("%s ", extension);
        }
        Logger::InfoT(LOG_TAG, extentions);

        if (Renderer::Check(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create Vulkan instance!");
        }
    }

    void Instance::CreateDebugCallback()
    {
#if defined(MANTIS_DEBUG)
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = &DebugCallback;
        debugCreateInfo.pUserData = nullptr;

        if (FvkCreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugCallback) != VK_SUCCESS)
        {
            Logger::ErrorT(LOG_TAG, "Failed to register Vulkan debug message callback!");
        }
#endif
    }
}