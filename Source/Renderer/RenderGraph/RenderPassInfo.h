#pragma once

#include "Mantis.h"

#include "Renderer/RendererConfig.h"

namespace Mantis
{
    enum struct RenderPassOp : int
    {
        None                    = 0,
        ClearDepthStencil       = 1 << 0,
        LoadDepthStencil        = 1 << 1,
        StoreDepthStencil       = 1 << 2,
        DepthStencilReadOnly    = 1 << 3,
        EnableTransientStore    = 1 << 4,
        EnableTransientLoad     = 1 << 5,
    };
    ENUM_IS_FLAGS(RenderPassOp);

    struct RenderPassInfo
    {
        ImageView* colorAttachments[RendererConfig::MAX_ATTACHMENTS];
        ImageView* depthStencil = nullptr;
        uint32_t numColorAttachments = 0;
        RenderPassOp opFlags = RenderPassOp::None;
        uint32_t clearAttachments = 0;
        uint32_t loadAttachments = 0;
        uint32_t storeAttachments = 0;
        uint32_t baseLayer = 0;
        uint32_t numLayers = 1;

        VkRect2D renderArea = { { 0, 0 }, { UINT32_MAX, UINT32_MAX } };

        VkClearColorValue clearColor[RendererConfig::MAX_ATTACHMENTS] = {};
        VkClearDepthStencilValue clearDepthStencil = { 1.0f, 0 };

        enum struct DepthStencilMode
        {
            None,
            ReadOnly,
            ReadWrite,
        };

        struct Subpass
        {
            uint32_t inputAttachments[RendererConfig::MAX_ATTACHMENTS];
            uint32_t colorAttachments[RendererConfig::MAX_ATTACHMENTS];
            uint32_t resolveAttachments[RendererConfig::MAX_ATTACHMENTS];
            unsigned numInputAttachments = 0;
            unsigned numColorAttachments = 0;
            unsigned numResolveAttachments = 0;
            DepthStencilMode depthStencilMode = DepthStencilMode::ReadWrite;
        };

        const Subpass* subpasses = nullptr;
        unsigned numSubpasses = 0;
    };

}
