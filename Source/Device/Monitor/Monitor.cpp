#include "stdafx.h"
#include "Monitor.h"

#define LOG_TAG MANTIS_TEXT("Monitor")

namespace Mantis
{
    eastl::vector<eastl::shared_ptr<Monitor>> Monitor::s_monitors;
    Delegate<void(Monitor*, bool)> Monitor::s_onMonitorConnect;

    void CallbackMonitor(GLFWmonitor* monitor, int event)
    {
        if (event == GLFW_CONNECTED)
        {
            auto& it = Monitor::s_monitors.emplace_back(eastl::make_unique<Monitor>(monitor));

            Logger::InfoTF(LOG_TAG, "Monitor \"%s\" connected", it->GetName().c_str());
            Monitor::s_onMonitorConnect(it.get(), true);
        }
        else if (event == GLFW_DISCONNECTED)
        {
            for (auto& m : Monitor::s_monitors)
            {
                if (m->GetMonitor() == monitor)
                {
                    Logger::InfoTF(LOG_TAG, "Monitor \"%s\" disconnected", m->GetName().c_str());
                    Monitor::s_onMonitorConnect(m.get(), false);
                }
            }

            Monitor::s_monitors.erase(eastl::remove_if(Monitor::s_monitors.begin(), Monitor::s_monitors.end(), [monitor](eastl::shared_ptr<Monitor>& m)
                {
                    return monitor == m->GetMonitor();
                }));
        }
    }

    void Monitor::DetectMonitors()
    {
        s_monitors.clear();

        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        for (int i = 0; i < monitorCount; i++)
        {
            auto& it = s_monitors.emplace_back(eastl::make_unique<Monitor>(monitors[i]));
            Logger::InfoTF(LOG_TAG, "Monitor \"%s\" detected", it->GetName().c_str());
        }
    }

    const Monitor* Monitor::GetPrimary()
    {
        for (const auto& monitor : s_monitors)
        {
            if (monitor->IsPrimary())
            {
                return monitor.get();
            }
        }
        return nullptr;
    }

    Monitor::Monitor(GLFWmonitor* monitor) :
        m_monitor(monitor)
    {
    }

    String Monitor::GetName() const
    {
        return glfwGetMonitorName(m_monitor);
    }

    bool Monitor::IsPrimary() const
    {
        return m_monitor == glfwGetPrimaryMonitor();
    }

    RectInt Monitor::GetWorkarea() const
    {
        RectInt result;
        glfwGetMonitorWorkarea(m_monitor, &result.x, &result.y, &result.width, &result.height);
        return result;
    }

    Vector2Int Monitor::GetPosition() const
    {
        Vector2Int result;
        glfwGetMonitorPos(m_monitor, &result.x, &result.y);
        return result;
    }

    Vector2Int Monitor::GetSize() const
    {
        Vector2Int result;
        glfwGetMonitorPhysicalSize(m_monitor, &result.x, &result.y);
        return result;
    }

    Vector2 Monitor::GetContentScale() const
    {
        Vector2 result;
        glfwGetMonitorContentScale(m_monitor, &result.x, &result.y);
        return result;
    }

    eastl::vector<VideoMode> Monitor::GetVideoModes() const
    {
        int videoModeCount;
        const GLFWvidmode* videoModes = glfwGetVideoModes(m_monitor, &videoModeCount);
        
        eastl::vector<VideoMode> modes(videoModeCount);
        for (int i = 0; i < videoModeCount; i++)
        {
            modes[i] = *reinterpret_cast<const VideoMode*>(&videoModes[i]);
        }

        return modes;
    }

    VideoMode Monitor::GetVideoMode() const
    {
        return *reinterpret_cast<const VideoMode*>(glfwGetVideoMode(m_monitor));
    }

    GammaRamp Monitor::GetGammaRamp() const
    {
        return *reinterpret_cast<const GammaRamp*>(glfwGetGammaRamp(m_monitor));
    }

    void Monitor::SetGammaRamp(const GammaRamp& gammaRamp) const
    {
        glfwSetGammaRamp(m_monitor, reinterpret_cast<const GLFWgammaramp*>(&gammaRamp));
    }
}