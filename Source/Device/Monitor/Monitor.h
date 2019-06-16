#pragma once

#include "Mantis.h"

namespace Mantis
{
    struct VideoMode
    {
    public:
        /// <summary>
        /// The width in screen coordinates.
        /// </summary>
        int32_t width;
        /// <summary>
        /// The height in screen coordinates.
        /// </summary>
        int32_t height;
        /// <summary>
        /// The bit depth of the red channel.
        /// </summary>
        int32_t redBits;
        /// <summary>
        /// The bit depth of the green channel.
        /// </summary>
        int32_t greenBits;
        /// <summary>
        /// The bit depth of the blue channel.
        /// </summary>
        int32_t blueBits;
        /// <summary>
        /// The refresh rate in Hz.
        /// </summary>
        int32_t refreshRate;
    };

    /// <summary>
    /// Describes the gamma repsonse curve of a monitor.
    /// </summary>
    class GammaRamp
    {
    public:
        /// <summary>
        /// An array of value describing the response of the red channel.
        /// </summary>
        uint16_t* red;
        /// <summary>
        /// An array of value describing the response of the green channel.
        /// </summary>
        uint16_t* green;
        /// <summary>
        /// An array of value describing the response of the blue channel.
        /// </summary>
        uint16_t* blue;
        /// <summary>
        /// The number of elements in each array.
        /// </summary>
        uint32_t size;
    };

    /// <summary>
    /// Represents a physical monitor.
    /// </summary>
    class Monitor
    {
    public:
        explicit Monitor(GLFWmonitor* monitor = nullptr);

        /// <summary>
        /// Gets the underlying monitor object.
        /// </summary>
        GLFWmonitor* GetMonitor() const { return m_monitor; }

        /// <summary>
        /// Gets the name of this monitor.
        /// </summary>
        String GetName() const;

        /// <summary>
        /// Gets if this is the primary monitor.
        /// </summary>
        bool IsPrimary() const;

        /// <summary>
        /// Gets the workarea of the monitor in pixels.
        /// </summary>
        RectInt GetWorkarea() const;

        /// <summary>
        /// Gets the position of the monitor's viewport on the virtual screen.
        /// </summary>
        Vector2Int GetPosition() const;

        /// <summary>
        /// Gets the content scale of the monitor, the ratio between the current DPI and the platform's default DPI.
        /// </summary>
        Vector2 GetContentScale() const;

        /// <summary>
        /// Gets the physical size of the display area in millimeters.
        /// </summary>
        Vector2Int GetSize() const;

        /// <summary>
        /// Gets the available video modes for this monitor.
        /// </summary>
        eastl::vector<VideoMode> GetVideoModes() const;

        /// <summary>
        /// Gets the current mode of this monitor.
        /// </summary>
        VideoMode GetVideoMode() const;

        /// <summary>
        /// Gets the current gamma ramp for this monitor.
        /// </summary>
        GammaRamp GetGammaRamp() const;

        /// <summary>
        /// Sets the current gamma ramp for this monitor.
        /// </summary>
        /// <param name="gammaRamp"></param>
        void SetGammaRamp(const GammaRamp& gammaRamp) const;

        /// <summary>
        /// Called when a monitor has been connected or disconnected.
        /// </summary>
        static Delegate<void(Monitor*, bool)>& OnMonitorConnect() { return s_onMonitorConnect; }

        /// <summary>
        /// Gets the current monitors.
        /// </summary>
        static void DetectMonitors();

        /// <summary>
        /// Gets the monitors that can be used with this window.
        /// </summary>
        static const eastl::vector<eastl::shared_ptr<Monitor>> GetMonitors() { return s_monitors; }

        /// <summary>
        /// Gets the main monitor.
        /// </summary>
        static const Monitor* GetPrimary();

    private:
        static eastl::vector<eastl::shared_ptr<Monitor>> s_monitors;
        static Delegate<void(Monitor*, bool)> s_onMonitorConnect;

        friend void CallbackMonitor(GLFWmonitor* monitor, int event);

        GLFWmonitor* m_monitor;
    };
}