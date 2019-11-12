#pragma once

#include "Mantis.h"

#include "Buffer.h"
#include "Renderer/Utils/Nameable.h"

namespace Mantis
{
    /// <summary>
    /// Manages a graphics buffer view.
    /// </summary>
    class BufferView
        : public NonCopyable
        , public Nameable
    {
    public:
        /// <summary>
        /// Creates a new buffer view.
        /// </summary>
        /// <param name="buffer">The buffer associated with this view.</param>
        /// <param name="format">The format of the buffer elements.</param>
        /// <param name="offset">The offset in bytes from the start of the buffer.</param>
        /// <param name="range">The size in bytes of the view into the buffer.</param>
        explicit BufferView(
            const Buffer* buffer,
            const VkFormat& format,
            const VkDeviceSize& offset = 0,
            const VkDeviceSize& range = VK_WHOLE_SIZE
        );

        /// <summary>
        /// Destroys the buffer view.
        /// </summary>
        virtual ~BufferView();

        /// <summary>
        /// Sets the name of this instance.
        /// </summary>
        void SetName(const String& name);

    private:
        VkBufferView m_view;
    };
}
