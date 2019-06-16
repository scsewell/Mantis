#include "stdafx.h"
#include "Window.h"

#include "Renderer/Renderer.h"
#include "Device/Graphics/Surface.h"
#include "Device/Monitor/Monitor.h"

#define LOG_TAG MANTIS_TEXT("Window")

namespace Mantis
{
    bool Window::s_initialized = false;
    eastl::vector<eastl::shared_ptr<Window>> Window::s_windows;


    void CallbackError(int error, const char* description)
    {
        Logger::ErrorTF(LOG_TAG, "%s (0x%08x)", description, error);
    }

    void CallbackPosition(GLFWwindow* window, int xpos, int ypos)
    {
        auto wnd = Window::GetWindow(window);

        if (!wnd->m_fullscreen)
        {
            wnd->m_position.x = xpos;
            wnd->m_position.y = ypos;
        }

        wnd->m_onPosition(wnd->m_position);
    }

    void CallbackSize(GLFWwindow* window, int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }

        auto wnd = Window::GetWindow(window);

        if (wnd->m_fullscreen)
        {
            wnd->m_fullscreenSize.x = width;
            wnd->m_fullscreenSize.y = height;
        }
        else
        {
            wnd->m_size.x = width;
            wnd->m_size.y = height;
        }

        wnd->m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        wnd->m_onSize(wnd->m_size);

        wnd->m_surface->UpdateCapabilities();
    }

    void CallbackClose(GLFWwindow* window)
    {
        auto wnd = Window::GetWindow(window);

        Logger::InfoTF(LOG_TAG, "Closing window \"%s\"...", wnd->m_title.c_str());

        wnd->m_onClose();
        wnd->m_closed = true;

        // remove the window from the window list
        auto it = eastl::find(Window::s_windows.begin(), Window::s_windows.end(), wnd);
        if (it != Window::s_windows.end())
        {
            Window::s_windows.erase(it);
        }
    }

    void CallbackFocus(GLFWwindow* window, int focused)
    {
        auto wnd = Window::GetWindow(window);

        wnd->m_focused = static_cast<bool>(focused);
        wnd->m_onFocus(focused == GLFW_TRUE);
    }

    void CallbackIconify(GLFWwindow* window, int iconified)
    {
        auto wnd = Window::GetWindow(window);

        wnd->m_iconified = iconified == GLFW_TRUE;
        wnd->m_onIconify(iconified == GLFW_TRUE);
    }

    void CallbackFrame(GLFWwindow* window, int width, int height)
    {
        auto wnd = Window::GetWindow(window);

        wnd->m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    }

    eastl::shared_ptr<Window> Window::GetWindow(GLFWwindow* window)
    {
        auto it = eastl::find_if(s_windows.begin(), s_windows.end(), [window](const eastl::shared_ptr<Window>& wnd)
            {
                return wnd->m_window == window;
            });

        if (it != s_windows.end())
        {
            return *it;
        }
        return nullptr;
    }


    bool Window::Init()
    {
        if (!s_initialized)
        {
            s_initialized = true;

            // Set the error error callback
            glfwSetErrorCallback(CallbackError);

            // Initialize the GLFW library
            Logger::InfoT(LOG_TAG, "Initializing GLFW...");

            if (glfwInit() == GLFW_FALSE)
            {
                Logger::ErrorT(LOG_TAG, "GLFW failed to initialize!");
                return false;
            }

            Logger::InfoTF(LOG_TAG, "GLFW version %s initialized", glfwGetVersionString());

            // Checks Vulkan support on GLFW
            if (glfwVulkanSupported() == GLFW_FALSE)
            {
                Logger::ErrorT(LOG_TAG, "GLFW failed to find Vulkan support!");
                return false;
            }

            Logger::InfoT(LOG_TAG, "Detecting monitors...");
            Monitor::DetectMonitors();

            // Set the monitor callback
            glfwSetMonitorCallback(CallbackMonitor);
        }
        return true;
    }

    void Window::Update()
    {
        glfwPollEvents();
    }

    void Window::Deinit()
    {
        if (s_initialized)
        {
            Logger::InfoT(LOG_TAG, "Deinit GLFW...");

            // destroy all windows
            for (auto& window : s_windows)
            {
                window->Destroy();
            }
            s_windows.clear();

            // terminate GLFW
            glfwTerminate();

            s_initialized = false;
        }
    }

    eastl::pair<const char**, uint32_t> Window::GetInstanceExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        return eastl::make_pair(glfwExtensions, glfwExtensionCount);
    }

    eastl::shared_ptr<Window> Window::Create()
    {
        // initialize the windowing system in case it has not been already
        Init();

        // initialize the renderer
        Renderer::InitStart();

        // create a window
        auto window = eastl::shared_ptr<Window>(new Window());

        if (window->m_window == nullptr)
        {
            Logger::ErrorT(LOG_TAG, "Failed to create window!");
            return nullptr;
        }

        // finish initialization of the renderer 
        Renderer::InitEnd(window->GetSurface());

        // keep track of the window
        s_windows.push_back(window);
        return window;
    }

    void Window::Close(eastl::shared_ptr<Window> window)
    {
        // remove the window from the window list
        auto it = eastl::find(s_windows.begin(), s_windows.end(), window);
        if (it != s_windows.end())
        {
            s_windows.erase(it);
        }

        // destory the window
        window->Destroy();
    }

    Window::Window() :
        m_window(nullptr),
        m_surface(nullptr),
        m_size(800, 600),
        m_fullscreenSize(0, 0),
        m_position(0, 0),
        m_title(MANTIS_PROJECT_NAME),
        m_borderless(false),
        m_resizable(true),
        m_floating(false),
        m_fullscreen(false),
        m_closed(false),
        m_focused(true),
        m_iconified(false)
    {
        Logger::InfoT(LOG_TAG, "Creating new window");

        m_aspectRatio = static_cast<float>(m_size.x) / static_cast<float>(m_size.y);

        VideoMode videoMode = Monitor::GetPrimary()->GetVideoMode();

        // Configures the window
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // the window will stay hidden until after creation
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable context creation
        glfwWindowHint(GLFW_STENCIL_BITS, 8); // fixes 16 bit stencil bits in macOS
        glfwWindowHint(GLFW_STEREO, GLFW_FALSE); // no stereo view

        // Create a windowed mode window
        m_window = glfwCreateWindow(
            m_size.x,
            m_size.y,
            m_title.c_str(),
            nullptr,
            nullptr
        );

        // Gets any window errors
        if (m_window == nullptr)
        {
            Logger::ErrorT(LOG_TAG, "GLFW failed to create the window!");
            return;
        }

        // Sets the user pointer
        glfwSetWindowUserPointer(m_window, this);

        // Window attributes that can change later
        glfwSetWindowAttrib(m_window, GLFW_DECORATED, m_borderless ? GLFW_FALSE : GLFW_TRUE);
        glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, m_resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwSetWindowAttrib(m_window, GLFW_FLOATING, m_floating ? GLFW_TRUE : GLFW_FALSE);

        // Center the window position
        m_position.x = (videoMode.width - m_size.x) / 2;
        m_position.y = (videoMode.height - m_size.y) / 2;
        glfwSetWindowPos(m_window, m_position.x, m_position.y);

        // Sets fullscreen if enabled
        if (m_fullscreen)
        {
            SetFullscreen(true);
        }

        // Shows the glfw window
        glfwShowWindow(m_window);

        // Sets the displays callbacks
        glfwSetWindowPosCallback(m_window, CallbackPosition);
        glfwSetWindowSizeCallback(m_window, CallbackSize);
        glfwSetWindowCloseCallback(m_window, CallbackClose);
        glfwSetWindowFocusCallback(m_window, CallbackFocus);
        glfwSetWindowIconifyCallback(m_window, CallbackIconify);
        glfwSetFramebufferSizeCallback(m_window, CallbackFrame);

        // create the surface
        m_surface = new Surface(Renderer::Get()->GetInstance(), Renderer::Get()->GetPhysicalDevice(), this);
    }

    void Window::Destroy()
    {
        if (!m_closed)
        {
            Logger::InfoTF(LOG_TAG, "Closing window \"%s\"...", m_title.c_str());

            // delete the surface
            delete m_surface;
            m_surface = nullptr;

            // destroy the window
            glfwDestroyWindow(m_window);

            m_onClose();
            m_closed = true;
        }
    }

    void Window::SetTitle(const String& title)
    {
        m_title = title;
        glfwSetWindowTitle(m_window, m_title.c_str());
        m_onTitle(m_title);
    }

    void Window::SetIcons(const eastl::vector<String>& filenames)
    {
        Logger::ErrorT(LOG_TAG, "Window::SetIcons TODO");

        eastl::vector<GLFWimage> icons;

        for (const auto& filename : filenames)
        {
            Vector2Int size;
            eastl::unique_ptr<uint8_t> data = nullptr;

            if (data == nullptr)
            {
                continue;
            }

            GLFWimage icon = {};
            icon.width = size.x;
            icon.height = size.y;
            icon.pixels = data.get();
            icons.emplace_back(icon);
        }

        glfwSetWindowIcon(m_window, static_cast<int32_t>(icons.size()), icons.data());
    }

    void Window::SetSize(const Vector2Int& size)
    {
        m_aspectRatio = static_cast<float>(m_size.x) / static_cast<float>(m_size.y);
        glfwSetWindowSize(m_window, m_size.x, m_size.y);
    }

    void Window::SetPosition(const Vector2Int& position)
    {
        glfwSetWindowPos(m_window, m_position.x, m_position.y);
    }

    void Window::SetBorderless(const bool& borderless)
    {
        m_borderless = borderless;
        glfwSetWindowAttrib(m_window, GLFW_DECORATED, m_borderless ? GLFW_FALSE : GLFW_TRUE);
        m_onBorderless(m_borderless);
    }

    void Window::SetResizable(const bool& resizable)
    {
        m_resizable = resizable;
        glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, m_resizable ? GLFW_TRUE : GLFW_FALSE);
        m_onResizable(m_resizable);
    }

    void Window::SetFloating(const bool& floating)
    {
        m_floating = floating;
        glfwSetWindowAttrib(m_window, GLFW_FLOATING, m_floating ? GLFW_TRUE : GLFW_FALSE);
        m_onFloating(m_floating);
    }

    void Window::SetFullscreen(const bool& fullscreen, Monitor* monitor)
    {
        m_fullscreen = fullscreen;

        auto selected = monitor != nullptr ? monitor : Monitor::GetPrimary();
        auto videoMode = selected->GetVideoMode();

        if (fullscreen)
        {
            Logger::InfoTF(LOG_TAG, "Window \"%s\" is going fullscreen", m_title.c_str());
            m_fullscreenSize.x = static_cast<int>(videoMode.width);
            m_fullscreenSize.y = static_cast<int>(videoMode.height);
            glfwSetWindowMonitor(m_window, selected->GetMonitor(), 0, 0, m_fullscreenSize.x, m_fullscreenSize.y, GLFW_DONT_CARE);
        }
        else
        {
            Logger::InfoTF(LOG_TAG, "Window \"%s\" is going windowed", m_title.c_str());
            m_position.x = (videoMode.width - m_size.x) / 2;
            m_position.y = (videoMode.height - m_size.y) / 2;
            glfwSetWindowMonitor(m_window, nullptr, m_position.x, m_position.y, m_size.x, m_size.y, GLFW_DONT_CARE);
        }
        
        m_onFullscreen(m_fullscreen);
    }

    void Window::SetIconified(const bool& iconify)
    {
        if (!m_iconified && iconify)
        {
            glfwIconifyWindow(m_window);
        }
        else if (m_iconified && !iconify)
        {
            glfwRestoreWindow(m_window);
        }
    }

    void Window::SwapBuffers()
    {
        glfwSwapBuffers(m_window);
    }

    VkResult Window::CreateSurface(const VkInstance& instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) const
    {
        return glfwCreateWindowSurface(instance, m_window, allocator, surface);
    }
}
