#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Describes how an attachment may be used in a subpass.
    /// </summary>
    enum struct AttachmentMode
    {
        /// <summary>
        /// This attachment corresponds to an input attachment in a fragment shader. The attachment
        /// must be bound to the relevant pipelines in a descriptor set.
        /// </summary>
        Input,
        /// <summary>
        /// This attachment corresponds to an output location in the shader.
        /// </summary>
        Color,
        /// <summary>
        /// This attachment corresponds to a color attachment which is the result of resolving a multisampled
        /// attachment. If used, requires that every Color type attachment in this subpass has a corresponding
        /// resolve attachment.
        /// </summary>
        Resolve,
        /// <summary>
        /// This attachment corresponds to a color depth attachment.
        /// </summary>
        Depth,
        /// <summary>
        /// This attachment corresponds to an attachment that is not used by this subpass, but whose contents
        /// must be preserved throughout the subpass.
        /// </summary>
        Preserve,
    };

    /// <summary>
    /// Describes how an attachment is used in a subpass.
    /// </summary>
    struct AttachmentRef
    {
        /// <summary>
        /// The binding index of the referenced attachment.
        /// </summary>
        uint32_t binding;
        /// <summary>
        /// Describes how an attachment may be used in a subpass.
        /// </summary>
        AttachmentMode mode;
    };

    /// <summary>
    /// Represents a subpass.
    /// </summary>
    class Subpass
    {
    public:
        /// <summary>
        /// Creates a new subpass.
        /// </summary>
        /// <param name="binding">The subpass binding index.</param>
        /// <param name="attachmentRefs">Describes which attachments are used in the subpass and how.</param>
        /// <param name="dependencies">The subpasses that must be completed before this subpass may be executed.</param>
        Subpass(
            const uint32_t& binding,
            eastl::vector<AttachmentRef> attachmentRefs,
            eastl::vector<Subpass*> dependencies = {}
        );

        /// <summary>
        /// The binding index of the subpass.
        /// </summary>
        const uint32_t& GetBinding() const { return m_binding; }

        /// <summary>
        /// The indices of the attachments used for this subpass.
        /// </summary>
        const eastl::vector<AttachmentRef>& GetAttachmentRefs() const { return m_attachmentRefs; }

        /// <summary>
        /// The supbasses.
        /// </summary>
        const eastl::vector<Subpass*>& GetDependencies() const { return m_dependencies; }

    private:
        uint32_t m_binding;
        eastl::vector<AttachmentRef> m_attachmentRefs;
        eastl::vector<Subpass*> m_dependencies;
    };
}
