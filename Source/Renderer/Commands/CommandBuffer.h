#pragma once

#include "Mantis.h"
#include "CommandPool.h"

namespace Mantis
{
    class CommandBuffer
    {
    public:
        /// <summary>
        /// Creates a new command buffer.
        /// </summary>
        /// <param name="queueType">The queue to run this command buffer on.</param>
        /// <param name="bufferLevel">The buffer level.</param>
        /// <param name="begin">If recording will start right away.</param>
        explicit CommandBuffer(const QueueType& queueType,
            const VkCommandBufferLevel& bufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY, const bool& begin = true);

        ~CommandBuffer();

        /// <summary>
        /// Gets the underlying command buffer instance.
        /// </summary>
        operator const VkCommandBuffer& () const { return m_commandBuffer; }

        /// <summary>
        /// Gets the underlying command buffer instance.
        /// </summary>
        const VkCommandBuffer& GetCommandBuffer() const { return m_commandBuffer; }

        /// <summary>
        /// Checks if this command buffer is recording.
        /// </summary>
        const bool& IsRecording() const { return m_recording; }

        /// <summary>
        /// Begins the recording state for this command buffer.
        /// </summary>
        /// <param name="usage">How this command buffer will be used.</param>
        void Begin(const VkCommandBufferUsageFlags& usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        /// <summary>
        /// Ends the recording state for this command buffer.
        /// </summary>
        void End();

        /// <summary>
        /// Submits the command buffer to the queue and will hold the current thread idle until it has finished.
        /// </summary>
        void SubmitIdle();

        /// <summary>
        /// Submits the command buffer.
        /// </summary>
        /// <param name="fence">An optional fence that is signaled once the command buffer has completed.</param>
        /// <param name="signalSemaphore">An optional that is signaled once the command buffer has been executed.</param>
        /// <param name="waitSemaphore">An optional semaphore that will waited upon before the command buffer is executed.</param>
        /// <param name="waitStage">The pipeline stages used to wait at when using the wait semaphore.</param>
        void Submit(VkFence fence = VK_NULL_HANDLE, const VkSemaphore& signalSemaphore = VK_NULL_HANDLE,
            const VkSemaphore& waitSemaphore = VK_NULL_HANDLE, const VkPipelineStageFlags& waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    private:
        eastl::shared_ptr<CommandPool> m_commandPool;

        VkCommandBuffer m_commandBuffer;
        QueueType m_queueType;
        bool m_recording;
    };
}