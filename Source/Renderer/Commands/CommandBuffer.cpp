#include "stdafx.h"
#include "CommandBuffer.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("CommandBuffer")

namespace Mantis
{
    CommandBuffer::CommandBuffer(const QueueType& queueType, const VkCommandBufferLevel& bufferLevel, const bool& begin) :
        m_commandPool(Renderer::Get()->GetCommandPool(queueType)),
        m_commandBuffer(VK_NULL_HANDLE),
        m_queueType(queueType),
        m_recording(false)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = *m_commandPool;
        allocateInfo.level = bufferLevel;
        allocateInfo.commandBufferCount = 1;

        if (Renderer::Check(vkAllocateCommandBuffers(*logicalDevice, &allocateInfo, &m_commandBuffer)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create command buffer!");
        }

        if (begin)
        {
            Begin();
        }
    }

    CommandBuffer::~CommandBuffer()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        vkFreeCommandBuffers(*logicalDevice, *m_commandPool, 1, &m_commandBuffer);
    }

    void CommandBuffer::Begin(const VkCommandBufferUsageFlags& usage)
    {
        if (!m_recording)
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = usage;

            if (Renderer::Check(vkBeginCommandBuffer(m_commandBuffer, &beginInfo)))
            {
                Logger::ErrorT(LOG_TAG, "Failed to begin recording command buffer!");
            }

            m_recording = true;
        }
    }

    void CommandBuffer::End()
    {
        if (!m_recording)
        {
            if (Renderer::Check(vkEndCommandBuffer(m_commandBuffer)))
            {
                Logger::ErrorT(LOG_TAG, "Failed to end recording command buffer!");
            }

            m_recording = false;
        }
    }

    void CommandBuffer::SubmitIdle()
    {
        if (m_recording)
        {
            End();
        }

        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer;

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence fence;
        if (Renderer::Check(vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, &fence)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create fence!");
        }

        if (Renderer::Check(vkResetFences(*logicalDevice, 1, &fence)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to reset fence!");
        }

        if (Renderer::Check(vkQueueSubmit(logicalDevice->GetQueue(m_queueType), 1, &submitInfo, fence)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to submit queue!");
        }

        if (Renderer::Check(vkWaitForFences(*logicalDevice, 1, &fence, VK_TRUE, eastl::numeric_limits<uint64_t>::max())))
        {
            Logger::ErrorT(LOG_TAG, "Failed to wait for fence!");
        }

        vkDestroyFence(*logicalDevice, fence, nullptr);
    }

    void CommandBuffer::Submit(const VkFence fence, const VkSemaphore& signalSemaphore, const VkSemaphore& waitSemaphore, const VkPipelineStageFlags& waitStage)
    {
        if (m_recording)
        {
            End();
        }

        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer;

        if (waitSemaphore != VK_NULL_HANDLE)
        {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &waitSemaphore;
            submitInfo.pWaitDstStageMask = &waitStage;
        }

        if (signalSemaphore != VK_NULL_HANDLE)
        {
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &signalSemaphore;
        }

        if (fence != VK_NULL_HANDLE)
        {
            if (Renderer::Check(vkResetFences(*logicalDevice, 1, &fence)))
            {
                Logger::ErrorT(LOG_TAG, "Failed to reset fence!");
            }
        }

        if (Renderer::Check(vkQueueSubmit(logicalDevice->GetQueue(m_queueType), 1, &submitInfo, fence)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to submit queue!");
        }
    }
}