#include "stdafx.h"
#include "Fence.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Fence")

namespace Mantis
{
    Fence::Fence(bool signaled)
        : m_fence(VK_NULL_HANDLE)
        , m_waitComplete(false)
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        
        if (signaled)
        {
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        if (Renderer::Check(vkCreateFence(*Renderer::Get()->GetLogicalDevice(), &info, nullptr, &m_fence)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create fence!");
        }
    }

    Fence::~Fence()
    {
        vkDestroyFence(*Renderer::Get()->GetLogicalDevice(), m_fence, nullptr);
    }

    void Fence::Wait()
    {
        if (Renderer::Check(vkWaitForFences(*Renderer::Get()->GetLogicalDevice(), 1, &m_fence, VK_TRUE, eastl::numeric_limits<uint64_t>::max())))
        {
            Logger::ErrorT(LOG_TAG, "Failed to wait for fence!");
        }
        else
        {
            m_waitComplete = true;
        }
    }

    bool Fence::Wait(uint64_t timeout)
    {
        if (Renderer::Check(vkWaitForFences(*Renderer::Get()->GetLogicalDevice(), 1, &m_fence, VK_TRUE, timeout)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to wait for fence!");
            return false;
        }
        else
        {
            m_waitComplete = true;
            return true;
        }
    }

    void Fence::Reset()
    {
        if (m_waitComplete)
        {
            vkResetFences(*Renderer::Get()->GetLogicalDevice(), 1, &m_fence);
        }
    }
}
