#include "stdafx.h"
#include "Buffer.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Buffer")

namespace Mantis
{
    Buffer::Buffer(
        const VkDeviceSize& size,
        const VkBufferUsageFlags& usage,
        const VmaMemoryUsage& memoryUsage,
        const void* data
    )
        : m_buffer(VK_NULL_HANDLE)
        , m_allocator(Renderer::Get()->GetAllocator())
        , m_allocation(VK_NULL_HANDLE)
        , m_memoryFlags(0)
        , m_size(size)
        , m_usage(usage)
        , m_mapMode(MapMode::None)
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = memoryUsage;
        allocCreateInfo.memoryTypeBits = 0;
        allocCreateInfo.pool = VK_NULL_HANDLE;

        CreateBuffer(allocCreateInfo, data);
    }

    Buffer::Buffer(
        const VkDeviceSize& size,
        const VkBufferUsageFlags& usage,
        const VkMemoryPropertyFlags& properties,
        const void* data
    )
        : m_buffer(VK_NULL_HANDLE)
        , m_allocator(Renderer::Get()->GetAllocator())
        , m_allocation(VK_NULL_HANDLE)
        , m_memoryFlags(0)
        , m_size(size)
        , m_usage(usage)
        , m_mapMode(MapMode::None)
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.requiredFlags = properties;
        allocCreateInfo.memoryTypeBits = 0;
        allocCreateInfo.pool = VK_NULL_HANDLE;

        CreateBuffer(allocCreateInfo, data);
    }

    Buffer::~Buffer()
    {
		Renderer::Get()->DestroyBuffer(m_buffer, m_allocation);
    }

    void Buffer::SetName(const String& name)
    {
        SetDebugName(name, VK_OBJECT_TYPE_BUFFER, (uint64_t)m_buffer);
    }

    void Buffer::Map(void** data, const MapMode& mode)
    {
        if (m_mapMode == MapMode::None)
        {
            m_mapMode = mode;

            if (Renderer::Check(vmaMapMemory(m_allocator, m_allocation, data)))
            {
                Logger::ErrorT(LOG_TAG, "Failed to map buffer!");
            }

            // update the host memory to reflect the current buffer contents
            if (HAS_FLAGS(m_mapMode, MapMode::Read) && HAS_NO_FLAG(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                vmaInvalidateAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);
            }
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Buffer already mapped!");
        }
    }

    void Buffer::Unmap()
    {
        if (m_mapMode != MapMode::None)
        {
            vmaUnmapMemory(m_allocator, m_allocation);

            // if the buffer was writen flush to make the changes visible
            if (HAS_FLAGS(m_mapMode, MapMode::Write) && HAS_NO_FLAG(m_memoryFlags, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                vmaFlushAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE);
            }

            m_mapMode = MapMode::None;
        }
        else
        {
            Logger::WarningT(LOG_TAG, "Buffer already unmapped!");
        }
    }

    void Buffer::CreateBuffer(const VmaAllocationCreateInfo& allocCreateInfo, const void* data)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();
        auto graphicsFamily = logicalDevice->GetGraphicsFamily();
        auto presentFamily = logicalDevice->GetPresentFamily();
        auto computeFamily = logicalDevice->GetComputeFamily();

        eastl::array<uint32_t, 3> queueFamily = { graphicsFamily, presentFamily, computeFamily };

        // create the buffer
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = m_size;
        bufferCreateInfo.usage = m_usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamily.size());
        bufferCreateInfo.pQueueFamilyIndices = queueFamily.data();

        VmaAllocationInfo allocInfo;
        if (Renderer::Check(vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocCreateInfo, &m_buffer, &m_allocation, &allocInfo)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create buffer!");
        }

        // get the properties of the memory the buffer is stored in
        m_memoryFlags = Renderer::Get()->GetPhysicalDevice()->GetMemoryPropertyFlags(allocInfo.memoryType);

        // if a pointer to the buffer data has been passed, map the buffer and copy over the data
        if (data != nullptr)
        {
            void* mapped;
            Map(&mapped, MapMode::Write);
            memcpy(mapped, data, static_cast<size_t>(m_size));
            Unmap();
        }
    }
}
