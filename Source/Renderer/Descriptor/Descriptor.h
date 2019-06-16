#pragma once

#include "Mantis.h"

namespace Mantis
{
    class OffsetSize
    {
    public:
        OffsetSize(const uint32_t& offset, const uint32_t& size) :
            m_offset(offset),
            m_size(size)
        {}

        const uint32_t& GetOffset() const { return m_offset; }

        const uint32_t& GetSize() const { return m_size; }

        bool operator == (const OffsetSize& other) const
        {
            return m_offset == other.m_offset && m_size == other.m_size;
        }

        bool operator != (const OffsetSize& other) const
        {
            return m_offset != other.m_offset || m_size != other.m_size;
        }

    private:
        uint32_t m_offset;
        uint32_t m_size;
    };

    class WriteDescriptorSet
    {
    public:
        WriteDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet, const VkDescriptorImageInfo& imageInfo) :
            m_writeDescriptorSet(writeDescriptorSet),
            m_imageInfo(eastl::make_unique<VkDescriptorImageInfo>(imageInfo))
        {
            m_writeDescriptorSet.pImageInfo = m_imageInfo.get();
        }

        WriteDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet, const VkDescriptorBufferInfo& bufferInfo) :
            m_writeDescriptorSet(writeDescriptorSet),
            m_bufferInfo(eastl::make_unique<VkDescriptorBufferInfo>(bufferInfo))
        {
            m_writeDescriptorSet.pBufferInfo = m_bufferInfo.get();
        }

        const VkWriteDescriptorSet& GetWriteDescriptorSet() const { return m_writeDescriptorSet; }

    private:
        VkWriteDescriptorSet m_writeDescriptorSet;
        eastl::unique_ptr<VkDescriptorImageInfo> m_imageInfo;
        eastl::unique_ptr<VkDescriptorBufferInfo> m_bufferInfo;
    };

    class Descriptor
    {
    public:
        Descriptor() = default;
        virtual ~Descriptor() = default;
        virtual WriteDescriptorSet GetWriteDescriptor(const uint32_t& binding, const VkDescriptorType& descriptorType, const eastl::optional<OffsetSize>& offsetSize) const = 0;
    };
}