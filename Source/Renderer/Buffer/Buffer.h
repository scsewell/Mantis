#pragma once

#include "Mantis.h"

#include "vk_mem_alloc.h"

namespace Mantis
{
    /// <summary>
    /// Describes the operations valid for a mapped resource.
    /// </summary>
    enum struct MapMode : int
    {
        Read        = (1 << 0),
        Write       = (1 << 1),
        ReadWrite   = Read | Write
    };

    ENUM_IS_FLAGS(MapMode)

    /// <summary>
    /// Represents a graphics buffer.
    /// </summary>
    class Buffer
    {
    public:
        /// <summary>
        /// Creates a new buffer.
        /// </summary>
        /// <param name="size">Size of the buffer in bytes.</param>
        /// <param name="usage">Usage flag bitmask for the buffer.</param>
        /// <param name="memoryUsage">Memory usage for this buffer.</param>
        /// <param name="data">The data that should be copied to the buffer after creation.</param>
        Buffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VmaMemoryUsage& memoryUsage, const void* data = nullptr);

        /// <summary>
        /// Creates a new buffer.
        /// </summary>
        /// <param name="size">Size of the buffer in bytes.</param>
        /// <param name="usage">Usage flag bitmask for the buffer.</param>
        /// <param name="properties">Memory properties for this buffer.</param>
        /// <param name="data">The data that should be copied to the buffer after creation.</param>
        Buffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VkMemoryPropertyFlags& properties, const void* data = nullptr);

        virtual ~Buffer();

        /// <summary>
        /// Maps this buffer for reading/writing.
        /// </summary>
        /// <param name="data">Returns the mapped buffer pointer.</param>
        /// <param name="mode">Describes the opeations to be done on the buffer.</param>
        void Map(void** data, const MapMode& mode);

        /// <summary>
        /// Unmapes this buffer.
        /// </summary>
        void Unmap();

        /// <summary>
        /// Gets the size of the buffer.
        /// </summary>
        /// <returns></returns>
        const VkDeviceSize& GetSize() const { return m_size; }

        /// <summary>
        /// Gets the buffer instance.
        /// </summary>
        const VkBuffer& GetBuffer() const { return m_buffer; }

        /// <summary>
        /// Gets the memory allocated for this buffer.
        /// </summary>
        const VmaAllocation& GetAllocation() const { return m_allocation; }

    protected:
        void CreateBuffer(const VkDeviceSize& size, const VkBufferUsageFlags& usage, const VmaAllocationCreateInfo& allocCreateInfo, const void* data);

        VkDeviceSize m_size;
        VkMemoryPropertyFlags m_memoryFlags;
        VkBuffer m_buffer;
        VmaAllocator m_allocator;
        VmaAllocation m_allocation;
        MapMode m_mapMode;
    };
}