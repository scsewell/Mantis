#include "stdafx.h"
#include "Semaphore.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Semaphore")

namespace Mantis
{
	Semaphore::Semaphore()
		: m_semaphore(VK_NULL_HANDLE)
	{
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (Renderer::Check(vkCreateSemaphore(*Renderer::Get()->GetLogicalDevice(), &info, nullptr, &m_semaphore)))
		{
			Logger::ErrorT(LOG_TAG, "Failed to create semaphore!");
		}
	}

	Semaphore::~Semaphore()
	{
		vkDestroySemaphore(*Renderer::Get()->GetLogicalDevice(), m_semaphore, nullptr);
	}
}
