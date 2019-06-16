#include "stdafx.h"
#include "DescriptorsHandler.h"

#include "Renderer/Renderer.h"

namespace Mantis
{
    DescriptorsHandler::DescriptorsHandler() :
        m_shader(nullptr),
        m_changed(false)
    {}

    DescriptorsHandler::DescriptorsHandler(const Pipeline& pipeline) :
        m_shader(pipeline.GetShader()),
        m_descriptorSet(std::make_unique<DescriptorSet>(pipeline)),
        m_changed(true)
    {}

    void DescriptorsHandler::Push(const String& descriptorName, UniformHandler& uniformHandler, const eastl::optional<OffsetSize>& offsetSize)
    {
        if (m_shader == nullptr)
        {
            return;
        }

        uniformHandler.Update(m_shader->GetUniformBlock(descriptorName));
        Push(descriptorName, uniformHandler.GetUniformBuffer(), offsetSize);
    }

    void DescriptorsHandler::Push(const String& descriptorName, StorageHandler& storageHandler, const eastl::optional<OffsetSize>& offsetSize)
    {
        if (m_shader == nullptr)
        {
            return;
        }

        storageHandler.Update(m_shader->GetUniformBlock(descriptorName));
        Push(descriptorName, storageHandler.GetStorageBuffer(), offsetSize);
    }

    void DescriptorsHandler::Push(const String& descriptorName, PushHandler& pushHandler, const eastl::optional<OffsetSize>& offsetSize)
    {
        if (m_shader == nullptr)
        {
            return;
        }

        pushHandler.Update(m_shader->GetUniformBlock(descriptorName));
    }

    bool DescriptorsHandler::Update(const Pipeline& pipeline)
    {
        if (m_shader != pipeline.GetShader())
        {
            m_shader = pipeline.GetShader();
            m_pushDescriptors = pipeline.IsPushDescriptors();
            m_descriptors.clear();
            m_writeDescriptorSets.clear();

            if (!m_pushDescriptors)
            {
                m_descriptorSet = std::make_unique<DescriptorSet>(pipeline);
            }

            m_changed = false;
            return false;
        }

        if (m_changed)
        {
            m_writeDescriptorSets.clear();
            m_writeDescriptorSets.reserve(m_descriptors.size());

            for (const auto& [descriptorName, descriptor] : m_descriptors)
            {
                auto writeDescriptorSet = descriptor.m_writeDescriptor.GetWriteDescriptorSet();
                writeDescriptorSet.dstSet = VK_NULL_HANDLE;

                writeDescriptorSet.dstSet = m_descriptorSet->GetDescriptorSet();

                m_writeDescriptorSets.emplace_back(writeDescriptorSet);
            }

            m_descriptorSet->Update(m_writeDescriptorSets);

            m_changed = false;
        }

        return true;
    }

    void DescriptorsHandler::BindDescriptor(const CommandBuffer& commandBuffer, const Pipeline& pipeline)
    {
        m_descriptorSet->BindDescriptor(commandBuffer);
    }
}
