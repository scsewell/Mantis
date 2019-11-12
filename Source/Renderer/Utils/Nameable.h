#pragma once

#include "Mantis.h"

#include "Renderer/Renderer.h"

namespace Mantis
{
    class Nameable
    {
    public:
        /// <summary>
        /// Sets the debug name of this instance.
        /// </summary>
        virtual void SetName(const String& name) = 0;

    protected:
        /// <summary>
        /// Sets the debug name of this instance.
        /// </summary>
        void SetDebugName(const String& name, const VkObjectType& type, const uint64_t& handle)
        {
#ifdef MANTIS_DEBUG
            VkDebugUtilsObjectNameInfoEXT info = {};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            info.objectType = type;
            info.objectHandle = handle;
            info.pObjectName = name.c_str();

            vkSetDebugUtilsObjectNameEXT(*Renderer::Get()->GetLogicalDevice(), &info);
#endif
        }
    };
}
