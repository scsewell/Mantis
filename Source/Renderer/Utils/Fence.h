#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Manages a fence, used for synchronization between the GPU and host.
    /// </summary>
    class Fence
        : public NonCopyable
    {
    public:
        /// <summary>
        /// Creates a new fence.
        /// </summary>
        /// <param name="signaled">Start in the signaled state.</param>
        explicit Fence(bool signaled);

        /// <summary>
        /// Destroys the fence.
        /// </summary>
        ~Fence();

        /// <summary>
        /// Gets the underlying fence instance.
        /// </summary>
        const VkFence& GetFence() const { return m_fence; }

        /// <summary>
        /// Waits on this fence.
        /// </summary>
        void Wait();

        /// <summary>
        /// Waits on this fence with a timeout.
        /// </summary>
        /// <param name="timeout">The timeout for the fence in nanoseconds.</param>
        /// <returns>False if the wait timed out.</returns>
        bool Wait(uint64_t timeout);

        /// <summary>
        /// Unsignals the fence.
        /// </summary>
        void Reset();

    private:
        VkFence m_fence;
        bool m_waitComplete;
    };
}
