#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Gets a string representing a vulkan result.
    /// </summary>
    /// <param name="result">The result to stringify.</param>
    /// <returns>A new string instance.</returns>
    static String ResultToString(const VkResult& result)
    {
        switch (result)
        {
            case VK_SUCCESS:
                return "Success";
            case VK_NOT_READY:
                return "A fence or query has not yet completed!";
            case VK_TIMEOUT:
                return "A wait operation has not completed in the specified time!";
            case VK_EVENT_SET:
                return "An event is signaled!";
            case VK_EVENT_RESET:
                return "An event is unsignaled!";
            case VK_INCOMPLETE:
                return "A return array was too small for the result!";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "A host memory allocation has failed!";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "A device memory allocation has failed!";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "Initialization of an object could not be completed for implementation-specific reasons!";
            case VK_ERROR_DEVICE_LOST:
                return "The logical or physical device has been lost!";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "Mapping of a memory object has failed!";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "A requested layer is not present or could not be loaded!";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "A requested extension is not supported!";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "A requested feature is not supported!";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible!";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "Too many objects of the type have already been created!";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "A requested format is not supported on this device!";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "A surface is no longer available!";
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                return "An allocation failed due to having no more space in the descriptor pool!";
            case VK_SUBOPTIMAL_KHR:
                return "A swapchain no longer matches the surface properties exactly, but can still be used!";
            case VK_ERROR_OUT_OF_DATE_KHR:
                return "A surface has changed in such a way that it is no longer compatible with the swapchain!";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                return "The display used by a swapchain does not use the same presentable image layout!";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API!";
            case VK_ERROR_VALIDATION_FAILED_EXT:
                return "A validation layer found an error!";
            default:
                return "Unknown Vulkan error!";
        }
    }

    /// <summary>
    /// Gets a string representing a vulkan format.
    /// </summary>
    /// <param name="result">The format to stringify.</param>
    /// <returns>A new string instance.</returns>
    static String FormatToString(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:       return "R4G4B4A4_UNORM_PACK16";
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:       return "B4G4R4A4_UNORM_PACK16";
            case VK_FORMAT_R5G6B5_UNORM_PACK16:         return "R5G6B5_UNORM_PACK16";
            case VK_FORMAT_B5G6R5_UNORM_PACK16:         return "B5G6R5_UNORM_PACK16";
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:       return "R5G5B5A1_UNORM_PACK16";
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:       return "B5G5R5A1_UNORM_PACK16";
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:       return "A1R5G5B5_UNORM_PACK16";
            case VK_FORMAT_R8G8B8A8_UNORM:              return "R8G8B8A8_UNORM";
            case VK_FORMAT_R8G8B8A8_SNORM:              return "R8G8B8A8_SNORM";
            case VK_FORMAT_R8G8B8A8_SRGB:               return "R8G8B8A8_SRGB";
            case VK_FORMAT_B8G8R8A8_UNORM:              return "B8G8R8A8_UNORM";
            case VK_FORMAT_B8G8R8A8_SNORM:              return "B8G8R8A8_SNORM";
            case VK_FORMAT_B8G8R8A8_SRGB:               return "B8G8R8A8_SRGB";
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:       return "A8B8G8R8_UNORM_PACK32";
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:       return "A8B8G8R8_SNORM_PACK32";
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:        return "A8B8G8R8_SRGB_PACK32";
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:    return "A2R10G10B10_UNORM_PACK32";
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:    return "A2B10G10R10_UNORM_PACK32";
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:     return "B10G11R11_UFLOAT_PACK32";
            case VK_FORMAT_R16G16B16A16_UNORM:          return "R16G16B16A16_UNORM";
            case VK_FORMAT_R16G16B16A16_SFLOAT:         return "R16G16B16A16_SFLOAT";
            default:                                    return "UNDEFINED";
        }
    }

    /// <summary>
    /// Gets a string representing a vulkan color space.
    /// </summary>
    /// <param name="result">The color space to stringify.</param>
    /// <returns>A new string instance.</returns>
    static String ColorSpaceToString(VkColorSpaceKHR colorSpace)
    {
        switch (colorSpace)
        {
            case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:             return "SRGB_NONLINEAR_KHR";
            case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:       return "DISPLAY_P3_NONLINEAR_EXT";
            case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:       return "EXTENDED_SRGB_LINEAR_EXT";
            case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT:              return "DCI_P3_LINEAR_EXT";
            case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:           return "DCI_P3_NONLINEAR_EXT";
            case VK_COLOR_SPACE_BT709_LINEAR_EXT:               return "BT709_LINEAR_EXT";
            case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:            return "BT709_NONLINEAR_EXT";
            case VK_COLOR_SPACE_BT2020_LINEAR_EXT:              return "BT2020_LINEAR_EXT";
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:               return "HDR10_ST2084_EXT";
            case VK_COLOR_SPACE_DOLBYVISION_EXT:                return "DOLBYVISION_EXT";
            case VK_COLOR_SPACE_HDR10_HLG_EXT:                  return "HDR10_HLG_EXT";
            case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:            return "ADOBERGB_LINEAR_EXT";
            case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:         return "ADOBERGB_NONLINEAR_EXT";
            case VK_COLOR_SPACE_PASS_THROUGH_EXT:               return "PASS_THROUGH_EXT";
            case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:    return "EXTENDED_SRGB_NONLINEAR_EXT";
            case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:             return "DISPLAY_NATIVE_AMD";
            default:                                            return "UNDEFINED";
        }
    }

    /// <summary>
    /// Gets a string representing a vulkan image layout.
    /// </summary>
    /// <param name="result">The layout to stringify.</param>
    /// <returns>A new string instance.</returns>
    static String LayoutToString(VkImageLayout layout)
    {
        switch (layout)
        {
            case VK_IMAGE_LAYOUT_GENERAL:
                return "GENERAL";
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return "COLOR_ATTACHMENT";
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                return "DEPTH_STENCIL_ATTACHMENT";
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                return "DEPTH_STENCIL_READ_ONLY";
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return "SHADER_READ_ONLY";
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                return "TRANSFER_SRC";
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                return "TRANSFER_DST";
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                return "PREINITIALIZED";
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                return "DEPTH_READ_ONLY_STENCIL_ATTACHMENT";
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                return "DEPTH_ATTACHMENT_STENCIL_READ_ONLY";
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                return "PRESENT";
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                return "SHARED_PRESENT";
            default:
                return "UNDEFINED";
        }
    }

    /// <summary>
    /// Gets a string representing vulkan access flags.
    /// </summary>
    /// <param name="result">The flags to stringify.</param>
    /// <returns>A new string instance.</returns>
    static String AccessFlagsToString(VkAccessFlags flags)
    {
        String result;

        if (flags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT)
            result += "INDIRECT_COMMAND_READ ";
        if (flags & VK_ACCESS_INDEX_READ_BIT)
            result += "INDEX_READ ";
        if (flags & VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)
            result += "VERTEX_ATTRIBUTE_READ ";
        if (flags & VK_ACCESS_UNIFORM_READ_BIT)
            result += "UNIFORM_READ ";
        if (flags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
            result += "INPUT_ATTACHMENT_READ ";
        if (flags & VK_ACCESS_SHADER_READ_BIT)
            result += "SHADER_READ ";
        if (flags & VK_ACCESS_SHADER_WRITE_BIT)
            result += "SHADER_WRITE ";
        if (flags & VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
            result += "COLOR_ATTACHMENT_READ ";
        if (flags & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            result += "COLOR_ATTACHMENT_WRITE ";
        if (flags & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)
            result += "DEPTH_STENCIL_READ ";
        if (flags & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
            result += "DEPTH_STENCIL_WRITE ";
        if (flags & VK_ACCESS_TRANSFER_READ_BIT)
            result += "TRANSFER_READ ";
        if (flags & VK_ACCESS_TRANSFER_WRITE_BIT)
            result += "TRANSFER_WRITE ";
        if (flags & VK_ACCESS_HOST_READ_BIT)
            result += "HOST_READ ";
        if (flags & VK_ACCESS_HOST_WRITE_BIT)
            result += "HOST_WRITE ";
        if (flags & VK_ACCESS_MEMORY_READ_BIT)
            result += "MEMORY_READ ";
        if (flags & VK_ACCESS_MEMORY_WRITE_BIT)
            result += "MEMORY_WRITE ";

        if (!result.empty())
        {
            result.pop_back();
        }
        else
        {
            result = "NONE";
        }

        return result;
    }

    /// <summary>
    /// Gets a string representing vulkan pipeline state flags.
    /// </summary>
    /// <param name="result">The flags to stringify.</param>
    /// <returns>A new string instance.</returns>
    static String StageFlagsToString(VkPipelineStageFlags flags)
    {
        String result;

        if (flags & VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
            result += "TOP_OF_PIPE ";
        if (flags & (VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT))
            result += "VERTEX ";
        if (flags & (VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT))
            result += "TESSELLATION ";
        if (flags & VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)
            result += "GEOMETRY ";
        if (flags & (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT))
            result += "DEPTH ";
        if (flags & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
            result += "FRAGMENT ";
        if (flags & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
            result += "COLOR ";
        if (flags & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
            result += "COMPUTE ";
        if (flags & VK_PIPELINE_STAGE_TRANSFER_BIT)
            result += "TRANSFER ";
        if (flags & VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
            result += "BOTTOM_OF_PIPE ";
        if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT)
            result += "GRAPHICS ";
        if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
            result += "ALL ";

        if (!result.empty())
        {
            result.pop_back();
        }
        else
        {
            result = "NONE";
        }

        return result;
    }
}
