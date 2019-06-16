#pragma once

#include "Mantis.h"

#include "Pipeline.h"
#include "Renderer/Commands/CommandBuffer.h"

namespace Mantis
{
    /**
     * @brief Class that represents a compute compute pipeline.
     */
    class PipelineCompute :
        public Pipeline
    {
    public:
        /**
         * Creates a new compute pipeline.
         * @param shaderStage The shader file that will be loaded.
         * @param defines A list of defines added to the top of each shader.
         * @param pushDescriptors If no actual descriptor sets are allocated but instead pushed.
         */
        explicit PipelineCompute(
            std::filesystem::path shaderStage,
            std::vector<Shader::Define> defines = {},
            const bool& pushDescriptors = false
        );

        ~PipelineCompute();

        const std::filesystem::path& GetShaderStage() const { return m_shaderStage; }

        const eastl::vector<Shader::Define>& GetDefines() const { return m_defines; }

        const bool& IsPushDescriptors() const override { return m_pushDescriptors; }

        void CmdRender(const CommandBuffer& commandBuffer, const Vector2Int& extent) const;

        const Shader* GetShader() const override { return m_shader.get(); }

        const VkDescriptorSetLayout& GetDescriptorSetLayout() const override { return m_descriptorSetLayout; }

        const VkDescriptorPool& GetDescriptorPool() const override { return m_descriptorPool; }

        const VkPipeline& GetPipeline() const override { return m_pipeline; }

        const VkPipelineLayout& GetPipelineLayout() const override { return m_pipelineLayout; }

        const VkPipelineBindPoint& GetPipelineBindPoint() const override { return m_pipelineBindPoint; }

    private:
        void CreateShaderProgram();

        void CreateDescriptorLayout();

        void CreateDescriptorPool();

        void CreatePipelineLayout();

        void CreatePipelineCompute();

        std::filesystem::path m_shaderStage;
        eastl::vector<Shader::Define> m_defines;
        bool m_pushDescriptors;

        eastl::unique_ptr<Shader> m_shader;

        VkShaderModule m_shaderModule{ VK_NULL_HANDLE };
        VkPipelineShaderStageCreateInfo m_shaderStageCreateInfo{};

        VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };

        VkPipeline m_pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
        VkPipelineBindPoint m_pipelineBindPoint;
    };
}