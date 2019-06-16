#include "stdafx.h"
#include "Buffer.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Buffer")

namespace Mantis
{
    Buffer::Buffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VmaMemoryUsage& memoryUsage, const void* data) :
        m_size(size),
        m_buffer(VK_NULL_HANDLE),
        m_allocator(Renderer::Get()->GetAllocator()),
        m_allocation(VK_NULL_HANDLE),
        m_mapMode({})
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = memoryUsage;
        allocCreateInfo.memoryTypeBits = 0;
        allocCreateInfo.pool = VK_NULL_HANDLE;

        CreateBuffer(size, usage, allocCreateInfo, data);
    }

    Buffer::Buffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties, const void* data) :
        m_size(size),
        m_buffer(VK_NULL_HANDLE),
        m_allocator(Renderer::Get()->GetAllocator()),
        m_allocation(VK_NULL_HANDLE)
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.requiredFlags = properties;
        allocCreateInfo.memoryTypeBits = 0;
        allocCreateInfo.pool = VK_NULL_HANDLE;

        CreateBuffer(size, usage, allocCreateInfo, data);
    }

    void Buffer::CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VmaAllocationCreateInfo& allocCreateInfo, const void* data)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();
        auto graphicsFamily = logicalDevice->GetGraphicsFamily();
        auto presentFamily = logicalDevice->GetPresentFamily();
        auto computeFamily = logicalDevice->GetComputeFamily();

        eastl::array<uint32_t, 3> queueFamily = { graphicsFamily, presentFamily, computeFamily };

        // create the buffer
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamily.size());
        bufferCreateInfo.pQueueFamilyIndices = queueFamily.data();

        VmaAllocationInfo allocInfo;
        if (Renderer::Check(vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocCreateInfo, &m_buffer, &m_allocation, &allocInfo)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to crete buffer!");
        }

        // get the properties of the memory the buffer is stored in
        m_memoryFlags = Renderer::Get()->GetPhysicalDevice()->GetMemoryPropertyFlags(allocInfo.memoryType);

        // if a pointer to the buffer data has been passed, map the buffer and copy over the data
        if (data != nullptr)
        {
            void* mapped;
            Map(&mapped, MapMode::Write);
            memcpy(mapped, data, size);
            Unmap();
        }
    }

    Buffer::~Buffer()
    {
        vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
    }

    void Buffer::Map(void** data, const MapMode& mode)
    {
        m_mapMode = mode;

        if (Renderer::Check(vmaMapMemory(m_allocator, m_allocation, data)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to map buffer!");
        }

        if (HAS_FLAG(m_mapMode, MapMode::Read) && (m_memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            vmaInvalidateAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);
        }
    }

    void Buffer::Unmap()
    {
        vmaUnmapMemory(m_allocator, m_allocation);

        if (HAS_FLAG(m_mapMode, MapMode::Write) && (m_memoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            vmaFlushAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);
        }
    }
}
