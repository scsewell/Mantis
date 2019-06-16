#include "stdafx.h"
#include "Surface.h"

#include "Renderer/Renderer.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Device/Window/Window.h"

#define LOG_TAG MANTIS_TEXT("Surface")

namespace Mantis
{
    /// <summary>
    /// The first supported format will be used.
    /// </summary>
    static eastl::vector<VkFormat> PREFERRED_FORMATS =
    {
        VK_FORMAT_R16G16B16A16_UNORM,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    };

    /// <summary>
    /// The first supported color space will be used.
    /// </summary>
    static eastl::vector<VkColorSpaceKHR> PREFERRED_COLOR_SPACES =
    {
        // specifies support for the display’s native color space. This matches the color space
        // expectations of AMD’s FreeSync2 standard, for displays supporting it.
        VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,

        // Extended sRGB
        VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,
        VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,

        // BT.2020 / Rec.2020
        VK_COLOR_SPACE_HDR10_ST2084_EXT,
        VK_COLOR_SPACE_DOLBYVISION_EXT,
        VK_COLOR_SPACE_HDR10_HLG_EXT,
        VK_COLOR_SPACE_BT2020_LINEAR_EXT,

        // DCI-p3 / Display P3
        VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
        VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
        VK_COLOR_SPACE_DCI_P3_LINEAR_EXT,

        // Adobe RGB
        VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
        VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,

        // BT.709 / Rec.709
        VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
        VK_COLOR_SPACE_BT709_LINEAR_EXT,

        // sRGB
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    Surface::Surface(const Instance* instance, const PhysicalDevice* physicalDevice, const Window* window) :
        m_instance(instance),
        m_physicalDevice(physicalDevice),
        m_window(window),
        m_surface(VK_NULL_HANDLE),
        m_capabilities({}),
        m_format({})
    {
        // creates the surface
        window->CreateSurface(*m_instance, nullptr, &m_surface);

        // get surface information
        UpdateCapabilities();

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(*m_physicalDevice, m_surface, &presentModeCount, nullptr);

        m_presentationModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(*m_physicalDevice, m_surface, &presentModeCount, m_presentationModes.data());

        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(*m_physicalDevice, m_surface, &surfaceFormatCount, nullptr);

        eastl::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(*m_physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.data());

        Logger::InfoT(LOG_TAG, "Available surface formats:");
        for (const auto& surfaceFormat : surfaceFormats)
        {
            Logger::InfoT(LOG_TAG, StringifyFormat(surfaceFormat));
        }

        // select the best surface format
        m_format = ChooseFormat(surfaceFormats);
        Logger::InfoTF(LOG_TAG, "Selecting format: %s", StringifyFormat(m_format).c_str());
    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(*m_instance, m_surface, nullptr);
    }

    void Surface::UpdateCapabilities()
    {
        if (Renderer::Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*m_physicalDevice, m_surface, &m_capabilities)))
        {
            Logger::ErrorT(LOG_TAG, "Failed to get surface capabilities!");
        }
    }

    VkSurfaceFormatKHR Surface::ChooseFormat(const eastl::vector<VkSurfaceFormatKHR>& surfaceFormats)
    {
        if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
        {
            // the surface has no preferred format, we can choose what we want
            VkSurfaceFormatKHR format = {};
            format.format = PREFERRED_FORMATS[0];
            format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            return format;
        }
        else
        {
            // try to use an HDR format with a suitable color space
            for (const auto& preferredFormat : PREFERRED_FORMATS)
            {
                for (const auto& preferredSpace : PREFERRED_COLOR_SPACES)
                {
                    for (const auto& format : surfaceFormats)
                    {
                        if (format.format == preferredFormat && format.colorSpace == preferredSpace)
                        {
                            return format;
                        }
                    }
                }
            }

            // In case none of the preferred formats are available, use the first supported format
            return surfaceFormats[0];
        }
    }

    String Surface::StringifyFormat(const VkSurfaceFormatKHR& format)
    {
        String result = String();

        switch (format.format)
        {
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:       result.append("R4G4B4A4_UNORM_PACK16 ");    break;
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:       result.append("B4G4R4A4_UNORM_PACK16 ");    break;
            case VK_FORMAT_R5G6B5_UNORM_PACK16:         result.append("R5G6B5_UNORM_PACK16 ");      break;
            case VK_FORMAT_B5G6R5_UNORM_PACK16:         result.append("B5G6R5_UNORM_PACK16 ");      break;
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:       result.append("R5G5B5A1_UNORM_PACK16 ");    break;
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:       result.append("B5G5R5A1_UNORM_PACK16 ");    break;
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:       result.append("A1R5G5B5_UNORM_PACK16 ");    break;
            case VK_FORMAT_R8G8B8A8_UNORM:              result.append("R8G8B8A8_UNORM ");           break;
            case VK_FORMAT_R8G8B8A8_SNORM:              result.append("R8G8B8A8_SNORM ");           break;
            case VK_FORMAT_R8G8B8A8_SRGB:               result.append("R8G8B8A8_SRGB ");            break;
            case VK_FORMAT_B8G8R8A8_UNORM:              result.append("B8G8R8A8_UNORM ");           break;
            case VK_FORMAT_B8G8R8A8_SNORM:              result.append("B8G8R8A8_SNORM ");           break;
            case VK_FORMAT_B8G8R8A8_SRGB:               result.append("B8G8R8A8_SRGB ");            break;
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:       result.append("A8B8G8R8_UNORM_PACK32 ");    break;
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:       result.append("A8B8G8R8_SNORM_PACK32 ");    break;
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:        result.append("A8B8G8R8_SRGB_PACK32 ");     break;
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:    result.append("A2R10G10B10_UNORM_PACK32 "); break;
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:    result.append("A2B10G10R10_UNORM_PACK32 "); break;
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:     result.append("B10G11R11_UFLOAT_PACK32 ");  break;
            case VK_FORMAT_R16G16B16A16_UNORM:          result.append("R16G16B16A16_UNORM ");       break;
            case VK_FORMAT_R16G16B16A16_SFLOAT:         result.append("R16G16B16A16_SFLOAT ");      break;
            default:
                result.append_sprintf("%u ", format.format);
                break;
        }

        switch (format.colorSpace)
        {
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:             result.append("SRGB_NONLINEAR_KHR");            break;
            case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:       result.append("DISPLAY_P3_NONLINEAR_EXT");      break;
            case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:       result.append("EXTENDED_SRGB_LINEAR_EXT");      break;
            case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT:              result.append("DCI_P3_LINEAR_EXT");             break;
            case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:           result.append("DCI_P3_NONLINEAR_EXT");          break;
            case VK_COLOR_SPACE_BT709_LINEAR_EXT:               result.append("BT709_LINEAR_EXT");              break;
            case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:            result.append("BT709_NONLINEAR_EXT");           break;
            case VK_COLOR_SPACE_BT2020_LINEAR_EXT:              result.append("BT2020_LINEAR_EXT");             break;
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:               result.append("HDR10_ST2084_EXT");              break;
            case VK_COLOR_SPACE_DOLBYVISION_EXT:                result.append("DOLBYVISION_EXT");               break;
            case VK_COLOR_SPACE_HDR10_HLG_EXT:                  result.append("HDR10_HLG_EXT");                 break;
            case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:            result.append("ADOBERGB_LINEAR_EXT");           break;
            case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:         result.append("ADOBERGB_NONLINEAR_EXT");        break;
            case VK_COLOR_SPACE_PASS_THROUGH_EXT:               result.append("PASS_THROUGH_EXT");              break;
            case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:    result.append("EXTENDED_SRGB_NONLINEAR_EXT");   break;
            case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:             result.append("DISPLAY_NATIVE_AMD");            break;
            default:
                result.append_sprintf("%u ", format.colorSpace);
                break;
        }

        return result;
    }
}
