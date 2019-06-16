#pragma once

#include "Mantis.h"

#include "Pipeline.h"
#include "Shader/Shader.h"
#include "Renderer/RenderStage.h"

namespace Mantis
{
    class ImageDepth;
    class Image2d;

    /// <summary>
    /// Class that represents a graphics pipeline.
    /// </summary>
    class PipelineGraphics :
        public Pipeline
    {
    public:
        enum class Mode
        {
            Polygon, Mrt
        };

        enum class Depth
        {
            None        = 0,
            Read        = 1,
            Write       = 2,
            ReadWrite   = Read | Write
        };

        /**
         * Creates a new pipeline.
         * @param stage The graphics stage this pipeline will be run on.
         * @param shaderStages The source files to load the pipeline shaders from.
         * @param vertexInputs The vertex inputs that will be used as a shaders input.
         * @param defines A list of defines added to the top of each shader.
         * @param mode The mode this pipeline will run in.
         * @param depthMode The depth read/write that will be used.
         * @param topology The topology of the input assembly.
         * @param polygonMode The polygon draw mode.
         * @param cullMode The vertex cull mode.
         * @param frontFace The direction to render faces.
         * @param pushDescriptors If no actual descriptor sets are allocated but instead pushed.
         */
        PipelineGraphics(
            Stage stage,
            eastl::vector<std::filesystem::path> shaderStages,
            eastl::vector<Shader::VertexInput> vertexInputs,
            eastl::vector<Shader::Define> defines = {},
            const Mode& mode = Mode::Polygon,
            const Depth& depthMode = Depth::ReadWrite,
            const VkPrimitiveTopology& topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            const VkPolygonMode& polygonMode = VK_POLYGON_MODE_FILL,
            const VkCullModeFlags& cullMode = VK_CULL_MODE_BACK_BIT,
            const VkFrontFace& frontFace = VK_FRONT_FACE_CLOCKWISE,
            const bool& pushDescriptors = false
        );

        ~PipelineGraphics();

        /**
         * Gets the depth stencil used in a stage.
         * @param stage The stage to get values from, if not provided the pipelines stage will be used.
         * @return The depth stencil that is found.
         */
        const ImageDepth* GetDepthStencil(const eastl::optional<uint32_t>& stage = eastl::nullopt) const;

        /**
         * Gets a image used in a stage by the index given to it in the renderpass.
         * @param index The renderpass Image index.
         * @param stage The stage to get values from, if not provided the pipelines stage will be used.
         * @return The image that is found.
         */
        const Image2d* GetImage(const uint32_t& index, const eastl::optional<uint32_t>& stage = eastl::nullopt) const;

        /**
         * Gets the render stage viewport.
         * @param stage The stage to get values from, if not provided the pipelines stage will be used.
         * @return The the render stage viewport.
         */
        RectInt GetRenderArea(const eastl::optional<uint32_t>& stage = eastl::nullopt) const;

        const Stage& GetStage() const { return m_stage; }

        const eastl::vector<std::filesystem::path>& GetShaderStages() const { return m_shaderStages; }

        const eastl::vector<Shader::VertexInput>& GetVertexInputs() const { return m_vertexInputs; }

        const eastl::vector<Shader::Define>& GetDefines() const { return m_defines; }

        const Mode& GetMode() const { return m_mode; }

        const Depth& GetDepth() const { return m_depth; }

        const VkPrimitiveTopology& GetTopology() const { return m_topology; }

        const VkPolygonMode& GetPolygonMode() const { return m_polygonMode; }

        const VkCullModeFlags& GetCullMode() const { return m_cullMode; }

        const VkFrontFace& GetFrontFace() const { return m_frontFace; }

        const bool& IsPushDescriptors() const override { return m_pushDescriptors; }

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

        void CreateAttributes();

        void CreatePipeline();

        void CreatePipelinePolygon();

        void CreatePipelineMrt();

        Stage m_stage;
        eastl::vector<std::filesystem::path> m_shaderStages;
        eastl::vector<Shader::VertexInput> m_vertexInputs;
        eastl::vector<Shader::Define> m_defines;
        Mode m_mode;
        Depth m_depth;
        VkPrimitiveTopology m_topology;
        VkPolygonMode m_polygonMode;
        VkCullModeFlags m_cullMode;
        VkFrontFace m_frontFace;
        bool m_pushDescriptors;

        eastl::unique_ptr<Shader> m_shader;

        eastl::vector<VkDynamicState> m_dynamicStates;

        eastl::vector<VkShaderModule> m_modules;
        eastl::vector<VkPipelineShaderStageCreateInfo> m_stages;

        VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };

        VkPipeline m_pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
        VkPipelineBindPoint m_pipelineBindPoint;

        VkPipelineVertexInputStateCreateInfo m_vertexInputStateCreateInfo{};
        VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyState{};
        VkPipelineRasterizationStateCreateInfo m_rasterizationState{};
        eastl::array<VkPipelineColorBlendAttachmentState, 1> m_blendAttachmentStates;
        VkPipelineColorBlendStateCreateInfo m_colourBlendState{};
        VkPipelineDepthStencilStateCreateInfo m_depthStencilState{};
        VkPipelineViewportStateCreateInfo m_viewportState{};
        VkPipelineMultisampleStateCreateInfo m_multisampleState{};
        VkPipelineDynamicStateCreateInfo m_dynamicState{};
        VkPipelineTessellationStateCreateInfo m_tessellationState{};
    };

    class PipelineGraphicsCreate
    {
    public:
        PipelineGraphicsCreate(
            std::vector<std::filesystem::path> shaderStages = {},
            std::vector<Shader::VertexInput> vertexInputs = {},
            std::vector<Shader::Define> defines = {},
            const PipelineGraphics::Mode& mode = PipelineGraphics::Mode::Polygon,
            const PipelineGraphics::Depth& depth = PipelineGraphics::Depth::ReadWrite,
            const VkPrimitiveTopology& topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            const VkPolygonMode& polygonMode = VK_POLYGON_MODE_FILL,
            const VkCullModeFlags& cullMode = VK_CULL_MODE_BACK_BIT,
            const VkFrontFace& frontFace = VK_FRONT_FACE_CLOCKWISE,
            const bool& pushDescriptors = false
        ) :
            m_shaderStages(eastl::move(shaderStages)),
            m_vertexInputs(eastl::move(vertexInputs)),
            m_defines(eastl::move(defines)),
            m_mode(mode),
            m_depth(depth),
            m_topology(topology),
            m_polygonMode(polygonMode),
            m_cullMode(cullMode),
            m_frontFace(frontFace),
            m_pushDescriptors(pushDescriptors)
        {}

        /// <summary>
        /// Creates a new pipeline.
        /// </summary>
        /// <param name="pipelineStage">The pipelines graphics stage.</param>
        /// <returns>The created graphics pipeline.</returns>
        PipelineGraphics* Create(const Pipeline::Stage& pipelineStage) const
        {
            return new PipelineGraphics(pipelineStage, m_shaderStages, m_vertexInputs, m_defines, m_mode, m_depth, m_topology, m_polygonMode, m_cullMode, m_frontFace,
                m_pushDescriptors);
        }

        const eastl::vector<std::filesystem::path>& GetShaderStages() const { return m_shaderStages; }

        const eastl::vector<Shader::VertexInput>& GetVertexInputs() const { return m_vertexInputs; }

        const eastl::vector<Shader::Define>& GetDefines() const { return m_defines; }

        const PipelineGraphics::Mode& GetMode() const { return m_mode; }

        const PipelineGraphics::Depth& GetDepth() const { return m_depth; }

        const VkPrimitiveTopology& GetTopology() const { return m_topology; }

        const VkPolygonMode& GetPolygonMode() const { return m_polygonMode; }

        const VkCullModeFlags& GetCullMode() const { return m_cullMode; }

        const VkFrontFace& GetFrontFace() const { return m_frontFace; }

        const bool& GetPushDescriptors() const { return m_pushDescriptors; }

    private:
        eastl::vector<std::filesystem::path> m_shaderStages;
        eastl::vector<Shader::VertexInput> m_vertexInputs;
        eastl::vector<Shader::Define> m_defines;

        PipelineGraphics::Mode m_mode;
        PipelineGraphics::Depth m_depth;
        VkPrimitiveTopology m_topology;
        VkPolygonMode m_polygonMode;
        VkCullModeFlags m_cullMode;
        VkFrontFace m_frontFace;
        bool m_pushDescriptors;
    };
}