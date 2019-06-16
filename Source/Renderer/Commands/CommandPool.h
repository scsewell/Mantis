#pragma once

#include "Mantis.h"

#include "Device/Graphics/LogicalDevice.h"

namespace Mantis
{
    class CommandPool
    {
    public:
        /// <summary>
        /// Creates a new command pool.
        /// </summary>
        /// <param name="queueType">The queue type this command pool can allocate commands for.</param>
        /// <param name="threadId">The thread this pool belongs to.</param>
        explicit CommandPool(const QueueType& queueType = QueueType::Graphics, const std::thread::id& threadId = std::this_thread::get_id());

        ~CommandPool();

        /// <summary>
        /// Gets the underlying command pool instance.
        /// </summary>
        operator const VkCommandPool& () const { return m_commandPool; }

        /// <summary>
        /// Gets the underlying command pool instance.
        /// </summary>
        const VkCommandPool& GetCommandPool() const { return m_commandPool; }

    private:
        VkCommandPool m_commandPool;
        QueueType m_queueType;
        std::thread::id m_threadId;
    };
}