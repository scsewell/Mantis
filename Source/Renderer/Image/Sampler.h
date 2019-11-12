#pragma once

#include "Mantis.h"

#include "Renderer/Utils/Nameable.h"

namespace Mantis
{
    struct SamplerCreateInfo
    {
        VkFilter magFilter;
        VkFilter minFilter;
        VkSamplerMipmapMode mipmapMode;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;
        VkSamplerAddressMode addressModeW;
        float mipLodBias;
        VkBool32 anisotropyEnable;
        float maxAnisotropy;
        VkBool32 compareEnable;
        VkCompareOp compareOp;
        float minLod;
        float maxLod;
        VkBorderColor borderColor;
        VkBool32 unnormalizedCoordinates;
    };

    /// <summary>
    /// Manages an image sampler.
    /// </summary>
    class Sampler
        : public NonCopyable
        , public Nameable
    {
    public:
        /// <summary>
        /// Creates a new sampler instance.
        /// </summary>
        /// <param name="createInfo">The creation parameters.</param>
        explicit Sampler(const SamplerCreateInfo& createInfo);

        /// <summary>
        /// Destroys the sampler.
        /// </summary>
        ~Sampler();

        /// <summary>
        /// Gets the underlying sampler.
        /// </summary>
        const VkSampler& GetSampler() const { return m_sampler; }

        /// <summary>
        /// Sets the name of this instance.
        /// </summary>
        void SetName(const String& name);

    private:
        VkSampler m_sampler;
    };
}
