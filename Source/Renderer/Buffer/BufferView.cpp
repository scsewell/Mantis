#include "stdafx.h"
#include "BufferView.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("BufferView")

namespace Mantis
{
    BufferView::BufferView(
        const Buffer* buffer,
        const VkFormat& format,
        const VkDeviceSize& offset,
        const VkDeviceSize& range
    )
        : m_view(VK_NULL_HANDLE)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkBufferViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        info.buffer = buffer->GetBuffer();
        info.format = format;
        info.offset = offset;
        info.range = range;

        if (Renderer::Check(vkCreateBufferView(*logicalDevice, &info, nullptr, &m_view)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create buffer view!");
        }
    }

    BufferView::~BufferView()
    {
        Renderer::Get()->DestroyBufferView(m_view);
    }

    void BufferView::SetName(const String& name)
    {
        SetDebugName(name, VK_OBJECT_TYPE_BUFFER_VIEW, (uint64_t)m_view);
    }
}
