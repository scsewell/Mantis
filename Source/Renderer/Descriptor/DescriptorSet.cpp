#include "stdafx.h"
#include "DescriptorSet.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Descriptor")

namespace Mantis
{
    DescriptorSet::DescriptorSet(const Pipeline& pipeline) :
        m_pipelineLayout(pipeline.GetPipelineLayout()),
        m_pipelineBindPoint(pipeline.GetPipelineBindPoint()),
        m_descriptorPool(pipeline.GetDescriptorPool()),
        m_descriptorSet(VK_NULL_HANDLE)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        VkDescriptorSetLayout layouts[1] = { pipeline.GetDescriptorSetLayout() };

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = layouts;

        if (Renderer::Check(vkAllocateDescriptorSets(*logicalDevice, &descriptorSetAllocateInfo, &m_descriptorSet)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create descriptor set!");
        }
    }

    DescriptorSet::~DescriptorSet()
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        if (Renderer::Check(vkFreeDescriptorSets(*logicalDevice, m_descriptorPool, 1, &m_descriptorSet)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to free descriptor set!");
        }
    }

    void DescriptorSet::Update(const eastl::vector<VkWriteDescriptorSet>& descriptorWrites)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();

        vkUpdateDescriptorSets(*logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void DescriptorSet::BindDescriptor(const CommandBuffer& commandBuffer)
    {
        vkCmdBindDescriptorSets(commandBuffer, m_pipelineBindPoint, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    }
}
