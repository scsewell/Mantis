// must be defined in only one cpp file
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "Renderer.h"

#define LOG_TAG MANTIS_TEXT("Renderer")

namespace Mantis
{
    void Renderer::InitStart()
    {
        if (!m_renderer)
        {
            m_renderer = eastl::make_unique<Renderer>();
        }
    }

    void Renderer::InitEnd(const Surface* surface)
    {
        if (m_renderer && !m_renderer->m_device)
        {
            m_renderer->CreateLogicalDevice(surface);
            m_renderer->CreateAllocator();
        }
    }

    void Renderer::Deinit()
    {
        if (m_renderer)
        {
            m_renderer.reset();
        }
    }

    Renderer::Renderer() :
        m_instance(eastl::make_unique<Instance>()),
        m_physicalDevice(eastl::make_unique<PhysicalDevice>(m_instance))
    {
    }

    Renderer::~Renderer()
    {
        vmaDestroyAllocator(m_allocator);
    }

    void Renderer::CreateLogicalDevice(const Surface* surface)
    {
        m_device = eastl::make_unique<LogicalDevice>(m_instance, m_physicalDevice, surface);
    }

    void Renderer::CreateAllocator()
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
        allocatorInfo.physicalDevice = *m_renderer->m_physicalDevice;
        allocatorInfo.device = *m_renderer->m_device;

        if (Renderer::Check(vmaCreateAllocator(&allocatorInfo, &m_allocator)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create vulkan memory allocator!");
        }
    }

    const eastl::shared_ptr<CommandPool>& Renderer::GetCommandPool(const QueueType& queueType, const std::thread::id& threadId)
    {
        switch (queueType)
        {
            case QueueType::Graphics:   return GetCommandPool(m_graphicsCommandPools, threadId);
            case QueueType::Compute:    return GetCommandPool(m_computeCommandPools,  threadId);;
            case QueueType::Transfer:   return GetCommandPool(m_transferCommandPools, threadId);;
            default:
                Logger::ErrorT(LOG_TAG, "Can't get command pool, unsupported queue type!");
                return nullptr;
        }
    }

    const eastl::shared_ptr<CommandPool>& Renderer::GetCommandPool(eastl::map<std::thread::id, eastl::shared_ptr<CommandPool>>& pools, const std::thread::id& threadId)
    {
        auto it = pools.find(threadId);
        if (it != pools.end())
        {
            return it->second;
        }

        pools.emplace(threadId, std::make_shared<CommandPool>(threadId));
        return pools.find(threadId)->second;
    }

    bool Renderer::Check(const VkResult& result)
    {
        if (result != VK_SUCCESS)
        {
            Logger::ErrorT(LOG_TAG, StringifyResult(result));
            return true;
        }
        return false;
    }

    String Renderer::StringifyResult(const VkResult& result)
    {
        switch (result)
        {
            case VK_SUCCESS:
                return "Success";
            case VK_NOT_READY:
                return "A fence or query has not yet completed!";
            case VK_TIMEOUT:
                return "A wait operation has not completed in the specified time!";
            case VK_EVENT_SET:
                return "An event is signaled!";
            case VK_EVENT_RESET:
                return "An event is unsignaled!";
            case VK_INCOMPLETE:
                return "A return array was too small for the result!";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "A host memory allocation has failed!";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "A device memory allocation has failed!";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "Initialization of an object could not be completed for implementation-specific reasons!";
            case VK_ERROR_DEVICE_LOST:
                return "The logical or physical device has been lost!";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "Mapping of a memory object has failed!";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "A requested layer is not present or could not be loaded!";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "A requested extension is not supported!";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "A requested feature is not supported!";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible!";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "Too many objects of the type have already been created!";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "A requested format is not supported on this device!";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "A surface is no longer available!";
            case VK_ERROR_OUT_OF_POOL_MEMORY:
               	return "An allocation failed due to having no more space in the descriptor pool!";
            case VK_SUBOPTIMAL_KHR:
                return "A swapchain no longer matches the surface properties exactly, but can still be used!";
            case VK_ERROR_OUT_OF_DATE_KHR:
                return "A surface has changed in such a way that it is no longer compatible with the swapchain!";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                return "The display used by a swapchain does not use the same presentable image layout!";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API!";
            case VK_ERROR_VALIDATION_FAILED_EXT:
                return "A validation layer found an error!";
            default:
                return "Unknown Vulkan error!";
        }
    }
}
