#pragma once

#include "Mantis.h"

#include "Renderer/Commands/CommandBuffer.h"
#include "Renderer/Pipeline/Pipeline.h"

namespace Mantis
{
    class Descriptor;
    class WriteDescriptorSet;

    class DescriptorSet
    {
    public:
        explicit DescriptorSet(const Pipeline& pipeline);

        ~DescriptorSet();

        void Update(const eastl::vector<VkWriteDescriptorSet>& descriptorWrites);

        void BindDescriptor(const CommandBuffer& commandBuffer);

        const VkDescriptorSet& GetDescriptorSet() const { return m_descriptorSet; }

    private:
        VkPipelineLayout m_pipelineLayout;
        VkPipelineBindPoint m_pipelineBindPoint;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSet m_descriptorSet;
    };
}
