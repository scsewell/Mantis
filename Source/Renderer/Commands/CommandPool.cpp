#include "stdafx.h"
#include "CommandPool.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("CommandPool")

namespace Mantis
{
    CommandPool::CommandPool(const QueueType& queueType, const std::thread::id& threadId) :
        m_commandPool(VK_NULL_HANDLE),
        m_queueType(queueType),
        m_threadId(threadId)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkCommandPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = logicalDevice->GetQueueFamilyIndex(m_queueType);
        
        if (Renderer::Check(vkCreateCommandPool(*logicalDevice, &createInfo, nullptr, &m_commandPool)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create command pool!");
        }
    }

    CommandPool::~CommandPool()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        vkDestroyCommandPool(*logicalDevice, m_commandPool, nullptr);
    }
}
