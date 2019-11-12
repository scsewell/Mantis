#pragma once

#include "Mantis.h"

namespace Mantis
{
    class Instance;
    class PhysicalDevice;
    class Window;

    class Surface
    {
    public:
        explicit Surface(const Instance* instance, const PhysicalDevice* physicalDevice, const Window* window);

        ~Surface();

        /// <summary>
        /// Gets the underlying surface.
        /// </summary>
        operator const VkSurfaceKHR& () const { return m_surface; }
        
        /// <summary>
        /// Gets the underlying surface.
        /// </summary>
        const VkSurfaceKHR& GetSurface() const { return m_surface; }

        /// <summary>
        /// Gets the capabilities of the surface.
        /// </summary>
        const VkSurfaceCapabilitiesKHR& GetCapabilities() const { return m_capabilities; }

        /// <summary>
        /// Gets the capabilities of the surface.
        /// </summary>
        const eastl::vector<VkPresentModeKHR>& GetPresentationModes() const { return m_presentationModes; }

        /// <summary>
        /// Gets the color format of the surface.
        /// </summary>
        const VkSurfaceFormatKHR& GetFormat() const { return m_format; }

        /// <summary>
        /// Updates the capabilities of the surface.
        /// </summary>
        void UpdateCapabilities();

    private:
        /// <summary>
        /// Selects the preferred surface format.
        /// </summary>
        /// <param name="surfaceFormats">A list of available formats.</param>
        /// <returns>The format to use.</returns>
        static VkSurfaceFormatKHR ChooseFormat(const eastl::vector<VkSurfaceFormatKHR>& surfaceFormats);

        const Instance* m_instance;
        const PhysicalDevice* m_physicalDevice;
        const Window* m_window;

        VkSurfaceKHR m_surface;
        VkSurfaceCapabilitiesKHR m_capabilities;
        eastl::vector<VkPresentModeKHR> m_presentationModes;
        VkSurfaceFormatKHR m_format;
    };
}
