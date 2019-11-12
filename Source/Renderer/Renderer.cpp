#include "stdafx.h"

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

    void Renderer::DestroyBuffer(const VkBuffer& buffer, const VmaAllocation& allocation)
    {
    }

    void Renderer::DestroyBufferView(const VkBufferView& view)
    {
    }

    void Renderer::DestroyImage(const VkImage& image, const VmaAllocation& allocation)
    {
    }

    void Renderer::DestroyImageView(const VkImageView& view)
    {
    }

    void Renderer::DestroySampler(const VkSampler& sampler)
    {
    }

    void Renderer::DestroyFramebuffer(const VkFramebuffer& framebuffer)
    {
    }

    void Renderer::DestroyPipeline(const VkPipeline& pipeline)
    {
    }

    bool Renderer::Check(const VkResult& result)
    {
        if (result != VK_SUCCESS)
        {
            Logger::ErrorT(LOG_TAG, ResultToString(result));
            return true;
        }
        return false;
    }
}
