#pragma once

#include "Mantis.h"

#include "Device/Window/Window.h"
#include "Device/Graphics/Instance.h"
#include "Device/Graphics/PhysicalDevice.h"
#include "Device/Graphics/LogicalDevice.h"
#include "Device/Graphics/Surface.h"

#include "Renderer/Utils/Stringify.h"
#include "Renderer/Commands/CommandPool.h"

#include "vk_mem_alloc.h"

namespace Mantis
{
    class Renderer
    {
    public:
        /// <summary>
        /// Gets the renderer instance. Will be null until a window has been created.
        /// </summary>
        static Renderer* Get() { return m_renderer.get(); }

        /// <summary>
        /// Gets the Vulkan instance.
        /// </summary>
        const Instance* GetInstance() const { return m_instance.get(); }

        /// <summary>
        /// Gets the physical device.
        /// </summary>
        const PhysicalDevice* GetPhysicalDevice() const { return m_physicalDevice.get(); }

        /// <summary>
        /// Gets the logical device.
        /// </summary>
        const LogicalDevice* GetLogicalDevice() const { return m_device.get(); }

        /// <summary>
        /// Gets the allocator instance.
        /// </summary>
        const VmaAllocator& GetAllocator() { return m_allocator; }

        /// <summary>
        /// Gets the command pool for the specified queue and current thread.
        /// </summary>
        /// <param name="queueType">The queue type the command pool is to be used for.</param>
        /// <param name="threadId">The current thread.</param>
        /// <returns>The command pool.</returns>
        const eastl::shared_ptr<CommandPool>& GetCommandPool(const QueueType& queueType, const std::thread::id& threadId = std::this_thread::get_id());

        void DestroyBuffer(const VkBuffer& buffer, const VmaAllocation& allocation);
        void DestroyBufferView(const VkBufferView& view);
        void DestroyImage(const VkImage& image, const VmaAllocation& allocation);
        void DestroyImageView(const VkImageView& view);
        void DestroySampler(const VkSampler& sampler);
        void DestroyFramebuffer(const VkFramebuffer& framebuffer);
        void DestroyPipeline(const VkPipeline& pipeline);

        /// <summary>
        /// Determines if an operation was successful and logs any appropriate errors.
        /// </summary>
        /// <param name="result">The result to check.</param>
        /// <returns>True if the operation failed.</returns>
        static bool Check(const VkResult& result);

    private:
        friend class Window;

        /// <summary>
        /// Does the first stage of initialization.
        /// </summary>
        static void InitStart();

        /// <summary>
        /// Does the final stage of initialization.
        /// </summary>
        static void InitEnd(const Surface* surface);

        /// <summary>
        /// Destroys the renderer.
        /// </summary>
        static void Deinit();

        static eastl::unique_ptr<Renderer> m_renderer;

        Renderer();
        ~Renderer();

        void CreateLogicalDevice(const Surface* surface);
        void CreateAllocator();

        /// <summary>
        /// Gets a command pool for a thread for a given queue.
        /// </summary>
        /// <param name="pools">The pools for a queue type.</param>
        /// <param name="threadId">The current thread.</param>
        /// <returns>A command pool.</returns>
        const eastl::shared_ptr<CommandPool>& GetCommandPool(eastl::map<std::thread::id, eastl::shared_ptr<CommandPool>>& pools, const std::thread::id& threadId);

        eastl::unique_ptr<Instance> m_instance;
        eastl::unique_ptr<PhysicalDevice> m_physicalDevice;
        eastl::unique_ptr<LogicalDevice> m_device;

        VmaAllocator m_allocator;

        eastl::map<std::thread::id, eastl::shared_ptr<CommandPool>> m_graphicsCommandPools;
        eastl::map<std::thread::id, eastl::shared_ptr<CommandPool>> m_computeCommandPools;
        eastl::map<std::thread::id, eastl::shared_ptr<CommandPool>> m_transferCommandPools;
    };
}
