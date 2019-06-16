#include "stdafx.h"
#include "Subpass.h"

#define LOG_TAG MANTIS_TEXT("Subpass")

namespace Mantis
{
    Subpass::Subpass(
        const uint32_t& binding,
        eastl::vector<AttachmentRef> attachmentRefs,
        eastl::vector<Subpass*> dependencies
    ) :
        m_binding(binding),
        m_attachmentRefs(eastl::move(attachmentRefs)),
        m_dependencies(eastl::move(dependencies))
    {
        for (const auto& dependency : dependencies)
        {
            if (binding < dependency->GetBinding())
            {
                Logger::ErrorTF(LOG_TAG, "Subpass with binding index %u depends on a subpass with a greater binding index (%u)!", binding, dependency->GetBinding());
            }
        }
    }
}
