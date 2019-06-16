#pragma once

#include "Mantis.h"

#include "Renderer/Descriptor/Descriptor.h"
#include "Renderer/Descriptor/DescriptorSet.h"
//#include "Renderer/Buffers/UniformHandler.h"
//#include "Renderer/Buffers/StorageHandler.h"
//#include "Renderer/Buffers/PushHandler.h"
//#include "Renderer/Pipelines/Shader.h"

#define LOG_TAG MANTIS_TEXT("Descriptor")

namespace Mantis
{
    /// <summary>
    /// Mananges a descriptor set.
    /// </summary>
    class DescriptorsHandler
    {
    public:
        DescriptorsHandler();

        explicit DescriptorsHandler(const Pipeline& pipeline);

        template<typename T>
        void Push(const String& descriptorName, const T& descriptor, const eastl::optional<OffsetSize>& offsetSize = eastl::nullopt)
        {
            if (m_shader == nullptr)
            {
                return;
            }

            // finds the local value given to the descriptor name
            auto it = m_descriptors.find(descriptorName);
            if (it != m_descriptors.end())
            {
                // if the descriptor and size have not changed then the write is not modified
                if (it->second.m_descriptor == *descriptor && it->second.m_offsetSize == offsetSize)
                {
                    return;
                }

                m_descriptors.erase(it);
            }

            // only non-null descriptors can be mapped
            if (*descriptor == nullptr)
            {
                return;
            }

            // when adding the descriptor find the location in the shader
            auto location = m_shader->GetDescriptorLocation(descriptorName);

            if (!location)
            {
#if defined(MANTIS_DEBUG)
                if (m_shader->ReportedNotFound(descriptorName, true))
                {
                    Logger::ErrorTF(LOG_TAG, "Could not find descriptor in shader \"%s\" of name \"%s\"!", m_shader->GetName().c_str(), descriptorName.c_str());
                }
#endif
                return;
            }

            auto descriptorType = m_shader->GetDescriptorType(*location);

            if (!descriptorType)
            {
#if defined(MANTIS_DEBUG)
                if (m_shader->ReportedNotFound(descriptorName, true))
                {
                    Logger::ErrorTF(LOG_TAG, "Could not find descriptor in shader \"%s\" of name \"%s\" at location \"%i\"!", m_shader->GetName().c_str(), descriptorName.c_str(), *location);
                }
#endif
                return;
            }

            // Adds the new descriptor value.
            auto writeDescriptor = ConstExpr::AsPtr(descriptor)->GetWriteDescriptor(*location, *descriptorType, offsetSize);
            m_descriptors.emplace(descriptorName, DescriptorValue{ ConstExpr::AsPtr(descriptor), std::move(writeDescriptor), offsetSize, *location });
            m_changed = true;
        }

        template<typename T>
        void Push(const String& descriptorName, const T& descriptor, WriteDescriptorSet writeDescriptorSet)
        {
            if (m_shader == nullptr)
            {
                return;
            }

            auto it = m_descriptors.find(descriptorName);
            if (it != m_descriptors.end())
            {
                m_descriptors.erase(it);
            }

            auto location = m_shader->GetDescriptorLocation(descriptorName);
            //auto descriptorType = m_shader->GetDescriptorType(*location);

            m_descriptors.emplace(descriptorName, DescriptorValue { *descriptor, eastl::move(writeDescriptorSet), eastl::nullopt, *location });
            m_changed = true;
        }

        void Push(const String& descriptorName, UniformHandler& uniformHandler, const eastl::optional<OffsetSize>& offsetSize = eastl::nullopt);

        void Push(const String& descriptorName, StorageHandler& storageHandler, const eastl::optional<OffsetSize>& offsetSize = eastl::nullopt);

        void Push(const String& descriptorName, PushHandler& pushHandler, const eastl::optional<OffsetSize>& offsetSize = eastl::nullopt);

        bool Update(const Pipeline& pipeline);

        void BindDescriptor(const CommandBuffer& commandBuffer, const Pipeline& pipeline);

        const DescriptorSet* GetDescriptorSet() const { return m_descriptorSet.get(); }

    private:
        struct DescriptorValue
        {
            const Descriptor* m_descriptor;
            WriteDescriptorSet m_writeDescriptor;
            eastl::optional<OffsetSize> m_offsetSize;
            uint32_t m_location;
        };

        const Shader* m_shader;
        eastl::unique_ptr<DescriptorSet> m_descriptorSet;

        eastl::map<String, DescriptorValue> m_descriptors;
        eastl::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
        bool m_changed;
    };
}