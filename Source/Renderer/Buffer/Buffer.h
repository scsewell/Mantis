#pragma once

#include "Mantis.h"

#include "vk_mem_alloc.h"

#include "Renderer/Utils/Nameable.h"

namespace Mantis
{
    /// <summary>
    /// Describes the operations valid for a mapped resource.
    /// </summary>
    enum struct MapMode : int
    {
        None        = 0,
        Read        = (1 << 0),
        Write       = (1 << 1),
        ReadWrite   = Read | Write
    };
    ENUM_IS_FLAGS(MapMode)

    /// <summary>
    /// Manages a graphics buffer.
    /// </summary>
    class Buffer
        : public NonCopyable
        , public Nameable
    {
    public:
        /// <summary>
        /// Creates a new buffer.
        /// </summary>
        /// <param name="size">Size of the buffer in bytes.</param>
        /// <param name="usage">Usage flag bitmask for the buffer.</param>
        /// <param name="memoryUsage">Memory usage for this buffer.</param>
        /// <param name="data">The data that should be copied to the buffer after creation.</param>
        explicit Buffer(
            const VkDeviceSize& size,
            const VkBufferUsageFlags& usage,
            const VmaMemoryUsage& memoryUsage,
            const void* data = nullptr
        );

        /// <summary>
        /// Creates a new buffer.
        /// </summary>
        /// <param name="size">Size of the buffer in bytes.</param>
        /// <param name="usage">Usage flag bitmask for the buffer.</param>
        /// <param name="properties">Memory properties for this buffer.</param>
        /// <param name="data">The data that should be copied to the buffer after creation.</param>
        explicit Buffer(
            const VkDeviceSize& size,
            const VkBufferUsageFlags& usage,
            const VkMemoryPropertyFlags& properties,
            const void* data = nullptr
        );

        /// <summary>
        /// Destroys the buffer.
        /// </summary>
        virtual ~Buffer();

        /// <summary>
        /// Gets the buffer instance.
        /// </summary>
        const VkBuffer& GetBuffer() const { return m_buffer; }

        /// <summary>
        /// Gets the size of the buffer in bytes.
        /// </summary>
        const VkDeviceSize& GetSize() const { return m_size; }

        /// <summary>
        /// Gets the usage of the buffer.
        /// </summary>
        const VkBufferUsageFlags& GetUsage() const { return m_usage; }

        /// <summary>
        /// Sets the name of this instance.
        /// </summary>
        void SetName(const String& name);

        /// <summary>
        /// Maps this buffer for reading/writing.
        /// </summary>
        /// <param name="data">Returns the mapped buffer pointer.</param>
        /// <param name="mode">Describes the opeations to be done on the buffer.</param>
        void Map(void** data, const MapMode& mode);

        /// <summary>
        /// Unmaps this buffer.
        /// </summary>
        void Unmap();

    protected:
        void CreateBuffer(const VmaAllocationCreateInfo& allocCreateInfo, const void* data);

        VkBuffer m_buffer;
        
        VmaAllocator m_allocator;
        VmaAllocation m_allocation;
        VkMemoryPropertyFlags m_memoryFlags;
        
        VkDeviceSize m_size;
        VkBufferUsageFlags m_usage;
        MapMode m_mapMode;
    };
}
