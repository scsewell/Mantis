#include "stdafx.h"
#include "UniformBuffer.h"

#include "Renderer/Renderer.h"

namespace Mantis
{
    UniformBuffer::UniformBuffer(const VkDeviceSize& size, const void* data) :
        Buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data)
    {}

    void UniformBuffer::Update(const void* data)
    {
        void* buffer;
        Buffer::Map(&buffer);
        memcpy(buffer, data, static_cast<size_t>(m_size));
        Buffer::Unmap();
    }

    VkDescriptorSetLayoutBinding UniformBuffer::GetDescriptorSetLayout(const uint32_t& binding, const VkDescriptorType& descriptorType, const VkShaderStageFlags& stage, const uint32_t& count)
    {
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
        descriptorSetLayoutBinding.binding = binding;
        descriptorSetLayoutBinding.descriptorType = descriptorType;
        descriptorSetLayoutBinding.descriptorCount = count;
        descriptorSetLayoutBinding.stageFlags = stage;
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

        return descriptorSetLayoutBinding;
    }

    WriteDescriptorSet UniformBuffer::GetWriteDescriptor(const uint32_t& binding, const VkDescriptorType& descriptorType, const eastl::optional<OffsetSize>& offsetSize) const
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = m_size;

        if (offsetSize)
        {
            bufferInfo.offset = offsetSize->GetOffset();
            bufferInfo.range = offsetSize->GetSize();
        }

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = VK_NULL_HANDLE;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = descriptorType;

        return WriteDescriptorSet(descriptorWrite, bufferInfo);
    }
}