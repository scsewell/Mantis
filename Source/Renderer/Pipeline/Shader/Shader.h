#pragma once

#include "Mantis.h"

namespace glslang
{
    class TProgram;
    class TType;
}

namespace Mantis
{
    /**
     * @brief Class that loads and processes a shader, and provides a reflection.
     */
    /// <summary>
    /// 
    /// </summary>
    class Shader
    {
    public:
        /**
         * A define added to the start of a shader, first value is the define name and second is the value to be set.
         */
        using Define = eastl::pair<String, String>;

        /// <summary>
        /// Class used to define sets of vertex inputs used in a shader.
        /// </summary>
        class VertexInput
        {
        public:
            VertexInput(eastl::vector<VkVertexInputBindingDescription> bindingDescriptions = {}, eastl::vector<VkVertexInputAttributeDescription> attributeDescriptions = {}) :
                m_binding(0),
                m_bindingDescriptions(eastl::move(bindingDescriptions)),
                m_attributeDescriptions(eastl::move(attributeDescriptions))
            {}

            const eastl::vector<VkVertexInputBindingDescription>& GetBindingDescriptions() const { return m_bindingDescriptions; }

            const eastl::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return m_attributeDescriptions; }

            bool operator < (const VertexInput& other) const
            {
                return m_bindingDescriptions.front().binding < other.m_bindingDescriptions.front().binding;
            }

        private:
            uint32_t m_binding;
            eastl::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
            eastl::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
        };

        class Uniform
        {
        public:
            explicit Uniform(const int32_t& binding = -1, const int32_t& offset = -1, const int32_t& size = -1, const int32_t& glType = -1, const bool& readOnly = false,
                const bool& writeOnly = false, const VkShaderStageFlags& stageFlags = 0) :
                m_binding(binding),
                m_offset(offset),
                m_size(size),
                m_glType(glType),
                m_readOnly(readOnly),
                m_writeOnly(writeOnly),
                m_stageFlags(stageFlags)
            {
            }

            const int32_t& GetBinding() const { return m_binding; }

            const int32_t& GetOffset() const { return m_offset; }

            const int32_t& GetSize() const { return m_size; }

            const int32_t& GetGlType() const { return m_glType; }

            const bool& IsReadOnly() const { return m_readOnly; }

            const bool& IsWriteOnly() const { return m_writeOnly; }

            const VkShaderStageFlags& GetStageFlags() const { return m_stageFlags; }

            bool operator == (const Uniform& other) const
            {
                return
                    m_binding == other.m_binding &&
                    m_offset == other.m_offset &&
                    m_size == other.m_size &&
                    m_glType == other.m_glType &&
                    m_readOnly == other.m_readOnly &&
                    m_writeOnly == other.m_writeOnly &&
                    m_stageFlags == other.m_stageFlags;
            }

            bool operator != (const Uniform& other) const
            {
                return !(*this == other);
            }

            String ToString() const
            {
                String str = String();
                str.append_sprintf("{binding:%i, offset:%i, size:%i, type:0x%06x}", m_binding, m_offset, m_size, m_glType);
                return str;
            }

        private:
            friend class Shader;

            int32_t m_binding;
            int32_t m_offset;
            int32_t m_size;
            int32_t m_glType;
            bool m_readOnly;
            bool m_writeOnly;
            VkShaderStageFlags m_stageFlags;
        };

        class UniformBlock
        {
        public:
            enum struct Type
            {
                None, Uniform, Storage, Push
            };

            explicit UniformBlock(const int32_t& binding = -1, const int32_t& size = -1, const VkShaderStageFlags& stageFlags = 0, const Type& type = Type::Uniform) :
                m_binding(binding),
                m_size(size),
                m_stageFlags(stageFlags),
                m_type(type)
            {
            }

            const int32_t& GetBinding() const { return m_binding; }

            const int32_t& GetSize() const { return m_size; }

            const VkShaderStageFlags& GetStageFlags() const { return m_stageFlags; }

            const Type& GetType() const { return m_type; }

            const eastl::map<String, Uniform>& GetUniforms() const { return m_uniforms; }

            eastl::optional<Uniform> GetUniform(const String& name) const
            {
                auto it = m_uniforms.find(name);
                if (it == m_uniforms.end())
                {
                    return eastl::nullopt;
                }
                return it->second;
            }

            bool operator == (const UniformBlock& other) const
            {
                return
                    m_binding == other.m_binding &&
                    m_size == other.m_size &&
                    m_stageFlags == other.m_stageFlags &&
                    m_type == other.m_type &&
                    m_uniforms == other.m_uniforms;
            }

            bool operator != (const UniformBlock& other) const
            {
                return !(*this == other);
            }

            String ToString() const
            {
                String str = String();
                str.append_sprintf("{binding:%i, size:%i, type:0x%02x}", m_binding, m_size, m_type);
                return str;
            }

        private:
            friend class Shader;

            int32_t m_binding;
            int32_t m_size;
            VkShaderStageFlags m_stageFlags;
            Type m_type;
            eastl::map<String, Uniform> m_uniforms;
        };

        class Attribute
        {
        public:
            explicit Attribute(const int32_t& set = -1, const int32_t& location = -1, const int32_t& size = -1, const int32_t& glType = -1) :
                m_set(set),
                m_location(location),
                m_size(size),
                m_glType(glType)
            {
            }
            
            const int32_t& GetSet() const { return m_set; }

            const int32_t& GetLocation() const { return m_location; }

            const int32_t& GetSize() const { return m_size; }

            const int32_t& GetGlType() const { return m_glType; }

            bool operator == (const Attribute& other) const
            {
                return 
                    m_set == other.m_set &&
                    m_location == other.m_location &&
                    m_size == other.m_size &&
                    m_glType == other.m_glType;
            }

            bool operator != (const Attribute& other) const
            {
                return !(*this == other);
            }

            String ToString() const
            {
                String str = String();
                str.append_sprintf("{set:%i, location:%i, size:%i, type:0x%06x}", m_set, m_location, m_size, m_glType);
                return str;
            }

        private:
            friend class Shader;

            int32_t m_set;
            int32_t m_location;
            int32_t m_size;
            int32_t m_glType;
        };

        class Constant
        {
        public:
            explicit Constant(const int32_t& binding = -1, const int32_t& size = -1, const VkShaderStageFlags& stageFlags = 0, const int32_t& glType = -1) :
                m_binding(binding),
                m_size(size),
                m_stageFlags(stageFlags),
                m_glType(glType)
            {
            }

            const int32_t& GetBinding() const { return m_binding; }

            const int32_t& GetSize() const { return m_size; }

            const VkShaderStageFlags& GetStageFlags() const { return m_stageFlags; }

            const int32_t& GetGlType() const { return m_glType; }

            bool operator == (const Constant& other) const
            {
                return 
                    m_binding == other.m_binding &&
                    m_size == other.m_size &&
                    m_stageFlags == other.m_stageFlags &&
                    m_glType == other.m_glType;
            }

            bool operator != (const Constant& other) const
            {
                return !(*this == other);
            }

            String ToString() const
            {
                String str = String();
                str.append_sprintf("{binding:%i, size:%i, stageFlags:0x%08x, type:0x%06x}", m_binding, m_size, m_stageFlags, m_glType);
                return str;
            }

        private:
            friend class Shader;

            int32_t m_binding;
            int32_t m_size;
            VkShaderStageFlags m_stageFlags;
            int32_t m_glType;
        };

        Shader();

        const String& GetName() const { return m_stages.back(); }

        bool ReportedNotFound(const String& name, const bool& reportIfFound) const;

        static VkFormat GlTypeToVk(const int32_t& type);

        eastl::optional<uint32_t> GetDescriptorLocation(const String& name) const;

        eastl::optional<uint32_t> GetDescriptorSize(const String& name) const;

        eastl::optional<Uniform> GetUniform(const String& name) const;

        eastl::optional<UniformBlock> GetUniformBlock(const String& name) const;

        eastl::optional<Attribute> GetAttribute(const String& name) const;

        eastl::vector<VkPushConstantRange> GetPushConstantRanges() const;

        const uint32_t& GetLastDescriptorBinding() const { return m_lastDescriptorBinding; }

        const eastl::map<String, Uniform>& GetUniforms() const { return m_uniforms; };

        const eastl::map<String, UniformBlock>& GetUniformBlocks() const { return m_uniformBlocks; };

        const eastl::map<String, Attribute>& GetAttributes() const { return m_attributes; };

        const eastl::map<String, Constant>& GetConstants() const { return m_constants; };

        const eastl::array<eastl::optional<uint32_t>, 3>& GetLocalSizes() const { return m_localSizes; }

        const eastl::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

        const eastl::vector<VkDescriptorPoolSize>& GetDescriptorPools() const { return m_descriptorPools; }

        eastl::optional<VkDescriptorType> GetDescriptorType(const uint32_t& location) const;

        const eastl::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return m_attributeDescriptions; }

        static VkShaderStageFlagBits GetShaderStage(const String& filename);

        VkShaderModule CreateShaderModule(const String& moduleName, const String& moduleCode, const String& preamble, const VkShaderStageFlags& moduleFlag);

        void CreateReflection();

        String ToString() const;

    private:
        static void IncrementDescriptorPool(eastl::map<VkDescriptorType, uint32_t>& descriptorPoolCounts, const VkDescriptorType& type);

        void LoadUniformBlock(const glslang::TProgram& program, const VkShaderStageFlags& stageFlag, const int32_t& i);

        void LoadUniform(const glslang::TProgram& program, const VkShaderStageFlags& stageFlag, const int32_t& i);

        void LoadVertexAttribute(const glslang::TProgram& program, const VkShaderStageFlags& stageFlag, const int32_t& i);

        static int32_t ComputeSize(const glslang::TType* ttype);

        eastl::vector<String> m_stages;
        eastl::map<String, Uniform> m_uniforms;
        eastl::map<String, UniformBlock> m_uniformBlocks;
        eastl::map<String, Attribute> m_attributes;
        eastl::map<String, Constant> m_constants;

        eastl::array<eastl::optional<uint32_t>, 3> m_localSizes;

        eastl::map<String, uint32_t> m_descriptorLocations;
        eastl::map<String, uint32_t> m_descriptorSizes;

        eastl::vector<VkDescriptorSetLayoutBinding> m_descriptorSetLayouts;
        uint32_t m_lastDescriptorBinding;
        eastl::vector<VkDescriptorPoolSize> m_descriptorPools;
        eastl::map<uint32_t, VkDescriptorType> m_descriptorTypes;
        eastl::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

        mutable eastl::vector<String> m_notFoundNames;
    };
}