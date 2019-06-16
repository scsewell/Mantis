#include "RenderStage.h"

#include "Renderer.h"
#include "Renderpass/Renderpass.h"
#include "Texture/ImageDepth.hpp"
#include "Device/Window/Window.h"

#define LOG_TAG MANTIS_TEXT("RenderStage")

namespace Mantis
{
    RenderStage::RenderStage(
        eastl::vector<Attachment> images,
        eastl::vector<Subpass> subpasses,
        const Viewport& viewport
    ) :
        m_attachments(eastl::move(images)),
        m_subpasses(eastl::move(subpasses)),
        m_viewport(viewport),
        m_subpassAttachmentCount(m_subpasses.size()),
        m_subpassMultisampled(m_subpasses.size())
    {
        for (const auto& attachment : m_attachments)
        {
            VkClearValue clearValue = {};

            switch (attachment.GetType())
            {
                case Attachment::Type::Image:
                    Color clearColor = attachment.GetClearColor();
                    clearValue.color.float32[0] = clearColor.r;
                    clearValue.color.float32[1] = clearColor.g;
                    clearValue.color.float32[2] = clearColor.b;
                    clearValue.color.float32[3] = clearColor.a;

                    for (const auto& subpass : m_subpasses)
                    {
                        auto attachmentRefs = subpass.GetAttachmentRefs();

                        if (eastl::find(attachmentRefs.begin(), attachmentRefs.end(), [&attachment](const AttachmentRef& attachmentRef) 
                            {
                                return attachment.GetBinding() == attachmentRef.binding;
                            }) != attachmentRefs.end())
                        {
                            m_subpassAttachmentCount[subpass.GetBinding()]++;

                            if (attachment.IsMultisampled())
                            {
                                m_subpassMultisampled[subpass.GetBinding()] = true;
                            }
                        }
                    }

                    break;
                case Attachment::Type::Depth:
                    clearValue.depthStencil.depth = 1.0f;
                    clearValue.depthStencil.stencil = 0;

                    m_depthAttachment = attachment;
                    break;
                case Attachment::Type::Swapchain:
                    Color clearColor = attachment.GetClearColor();
                    clearValue.color.float32[0] = clearColor.r;
                    clearValue.color.float32[1] = clearColor.g;
                    clearValue.color.float32[2] = clearColor.b;
                    clearValue.color.float32[3] = clearColor.a;

                    m_swapchainAttachment = attachment;
                    break;
            }

            m_clearValues.emplace_back(clearValue);
        }
    }

    void RenderStage::Update()
    {
        auto lastRenderArea = m_renderArea;

        m_renderArea.SetOffset(m_viewport.GetOffset());

        if (m_viewport.GetSize())
        {
            m_renderArea.SetExtent(m_viewport.GetScale() * *m_viewport.GetSize());
        }
        else
        {
            m_renderArea.SetExtent(m_viewport.GetScale() * Window::Get()->GetSize());
        }

        m_renderArea.SetAspectRatio(static_cast<float>(m_renderArea.GetExtent().m_x) / static_cast<float>(m_renderArea.GetExtent().m_y));
        m_renderArea.SetExtent(m_renderArea.GetExtent() + m_renderArea.GetOffset());

        m_outOfDate = m_renderArea != lastRenderArea;
    }

    void RenderStage::Rebuild(const Swapchain& swapchain)
    {
#if defined(MANTIS_DEBUG)
        auto startTime = Timer::Now();
#endif

        Update();

        auto physicalDevice = Renderer::Get()->GetPhysicalDevice();
        auto msaaSamples = physicalDevice->GetMsaaSamples();

        if (m_depthAttachment)
        {
            m_depthStencil = std::make_unique<ImageDepth>(m_renderArea.Size(), m_depthAttachment->IsMultisampled() ? msaaSamples : VK_SAMPLE_COUNT_1_BIT);
        }

        if (m_renderpass == nullptr)
        {
            auto surface = Renderer::Get()->GetSurface();
            m_renderpass = std::make_unique<Renderpass>(*this, m_depthStencil->GetFormat(), surface->GetFormat().format, msaaSamples);
        }

        m_framebuffers = std::make_unique<Framebuffers>(m_renderArea.Size(), *this, *m_renderpass, swapchain, *m_depthStencil, msaaSamples);

        m_descriptors.clear();

        for (const auto& image : m_attachments)
        {
            if (image.GetType() == Attachment::Type::Depth)
            {
                m_descriptors.emplace(image.GetName(), m_depthStencil.get());
            }
            else
            {
                m_descriptors.emplace(image.GetName(), m_framebuffers->GetAttachment(image.GetBinding()));
            }
        }

        m_outOfDate = false;

#if defined(MANTIS_DEBUG)
        Logger::DebugTF(LOG_TAG, "Render stage built in %.3fms", (Timer::Now() - startTime).AsMilliseconds<float>());
#endif
    }

    eastl::optional<Attachment> RenderStage::GetAttachment(const String& name) const
    {
        auto it = eastl::find_if(m_attachments.begin(), m_attachments.end(), [name](const Attachment& a)
        {
            return a.GetName() == name;
        });
        if (it == m_attachments.end())
        {
            return eastl::nullopt;
        }
        return *it;
    }

    eastl::optional<Attachment> RenderStage::GetAttachment(const uint32_t& binding) const
    {
        auto it = eastl::find_if(m_attachments.begin(), m_attachments.end(), [binding](const Attachment& a)
        {
            return a.GetBinding() == binding;
        });
        if (it == m_attachments.end())
        {
            return eastl::nullopt;
        }
        return *it;
    }

    const Descriptor* RenderStage::GetDescriptor(const String& name) const
    {
        auto it = m_descriptors.find(name);
        if (it != m_descriptors.end())
        {
            return it->second;
        }
        return nullptr;
    }

    const VkFramebuffer& RenderStage::GetFramebuffer(const uint32_t& index) const
    {
        if (index > m_framebuffers->GetFramebuffers().size())
        {
            return m_framebuffers->GetFramebuffers().at(0);
        }
        return m_framebuffers->GetFramebuffers().at(index);
    }
}
