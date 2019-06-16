#pragma once

#include "Mantis.h"

#include "Renderer/Commands/CommandBuffer.h"

namespace Mantis
{
    class Shader;

    /// <summary>
    /// Represents a pipeline.
    /// </summary>
    class Pipeline
    {
    public:
        /// <summary>
        /// Represents position in the render structure, first value being the renderpass and second for subpass.
        /// </summary>
        using Stage = eastl::pair<uint32_t, uint32_t>;

        Pipeline() = default;

        virtual ~Pipeline() = default;

        void BindPipeline(const CommandBuffer& commandBuffer) const
        {
            vkCmdBindPipeline(commandBuffer, GetPipelineBindPoint(), GetPipeline());
        }

        virtual const Shader* GetShader() const = 0;

        virtual const bool& IsPushDescriptors() const = 0;

        virtual const VkDescriptorSetLayout& GetDescriptorSetLayout() const = 0;

        virtual const VkDescriptorPool& GetDescriptorPool() const = 0;

        virtual const VkPipeline& GetPipeline() const = 0;

        virtual const VkPipelineLayout& GetPipelineLayout() const = 0;

        virtual const VkPipelineBindPoint& GetPipelineBindPoint() const = 0;
    };
}