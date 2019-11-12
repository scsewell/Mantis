#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Manages core configuration of the renderer.
    /// </summary>
    struct RendererConfig
    {
        /// <summary>
        /// The maxiumum number of attachments that may be simultanously used.
        /// </summary>
        static const uint32_t MAX_ATTACHMENTS = 8;

        /// <summary>
        /// Combine renderpasses into subpasses where possible.
        /// </summary>
        bool mergeSubpasses = true;
        /// <summary>
        /// Color rendertextures are transient.
        /// </summary>
        bool useTransientColor = true;
        /// <summary>
        /// Depth rendertextures are transient.
        /// </summary>
        bool useTransientDepthStencil = true;
        /// <summary>
        /// Uses a separate queue for compute.
        /// </summary>
        bool useAsyncComputePost = true;
        /// <summary>
        /// Forces using a unified queue.
        /// </summary>
        bool renderGraphForceSingleQueue = false;

        static RendererConfig& Get()
        {
            static RendererConfig config;
            return config;
        }
    };
}
