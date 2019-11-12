#pragma once

#include "Mantis.h"

namespace Mantis
{
	/// <summary>
	/// Manages a semaphore, used for synchronization between the GPU queues.
	/// </summary>
	class Semaphore
		: public NonCopyable
	{
	public:
		/// <summary>
		/// Creates a new semaphore.
		/// </summary>
		explicit Semaphore();

		/// <summary>
		/// Destroys the semaphore.
		/// </summary>
		~Semaphore();

		/// <summary>
		/// Gets the underlying fence instance.
		/// </summary>
		const VkSemaphore& GetSemaphore() const { return m_semaphore; }

	private:
		VkSemaphore m_semaphore;
	};
}
