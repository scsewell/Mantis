#pragma once

#include "Mantis.h"

#include "Buffer.h"
#include "Renderer/Descriptor/Descriptor.h"

namespace Mantis
{
    class UniformBuffer :
        public Descriptor,
        public Buffer
    {
    public:
        explicit UniformBuffer(const VkDeviceSize& size, const void* data = nullptr);

        /// <summary>
        /// Updates the contents of this buffer.
        /// </summary>
        /// <param name="data">The data to copy in.</param>
        void Update(const void* data);

        WriteDescriptorSet GetWriteDescriptor(const uint32_t& binding, const VkDescriptorType& descriptorType, const eastl::optional<OffsetSize>& offsetSize) const override;

        static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(const uint32_t& binding, const VkDescriptorType& descriptorType, const VkShaderStageFlags& stage, const uint32_t& count);
    };
}