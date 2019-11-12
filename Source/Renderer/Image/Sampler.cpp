#include "stdafx.h"
#include "Sampler.h"

#include "Renderer/Renderer.h"

#define LOG_TAG MANTIS_TEXT("Sampler")

namespace Mantis
{
    Sampler::Sampler(const SamplerCreateInfo& createInfo)
    {
        auto logicalDevice = Renderer::Get()->GetLogicalDevice();
        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();

        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = createInfo.magFilter;
        info.minFilter = createInfo.minFilter;
        info.mipmapMode = createInfo.mipmapMode;
        info.addressModeU = createInfo.addressModeU;
        info.addressModeV = createInfo.addressModeV;
        info.addressModeW = createInfo.addressModeW;
        info.mipLodBias = createInfo.mipLodBias;

        // check if anisotropy is available and use the supported filtering value closest to what is requested
        if (createInfo.anisotropyEnable && logicalDevice->GetEnabledFeatures().samplerAnisotropy)
        {
            info.anisotropyEnable = VK_TRUE;
            info.maxAnisotropy = eastl::min(eastl::max(createInfo.maxAnisotropy, 1.0f), physicalDevice->GetProperties().limits.maxSamplerAnisotropy);
        }
        else
        {
            info.anisotropyEnable = VK_FALSE;
        }

        info.compareEnable = createInfo.compareEnable;
        info.compareOp = createInfo.compareOp;
        info.minLod = createInfo.minLod;
        info.maxLod = createInfo.maxLod;
        info.borderColor = createInfo.borderColor;
        info.unnormalizedCoordinates = createInfo.unnormalizedCoordinates;

        if (Renderer::Check(vkCreateSampler(*logicalDevice, &info, nullptr, &m_sampler)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to create sampler!");
        }
    }

    Sampler::~Sampler()
    {
        Renderer::Get()->DestroySampler(m_sampler);
    }

    void Sampler::SetName(const String& name)
    {
        SetDebugName(name, VK_OBJECT_TYPE_SAMPLER, (uint64_t)m_sampler);
    }
}
