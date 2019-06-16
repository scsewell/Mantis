#pragma once

#include "Mantis.h"
#include "Utils/Delegate.h"

namespace Mantis
{
    class Monitor;
    class Surface;

    /// <summary>
    /// Manages a window that can be drawn to.
    /// </summary>
    class Window
    {
    public:
        /// <summary>
        /// Initializes the windowing system.
        /// </summary>
        static bool Init();

        /// <summary>
        /// Updates the windowing system.
        /// </summary>
        static void Update();

        /// <summary>
        /// Cleans up the windowing system.
        /// </summary>
        static void Deinit();

        /// <summary>
        /// Gets the vulkan extentions required by the windowing API.
        /// </summary>
        static eastl::pair<const char**, uint32_t> GetInstanceExtensions();

        /// <summary>
        /// Creates a window.
        /// </summary>
        static eastl::shared_ptr<Window> Create();

        /// <summary>
        /// Destroys a window.
        /// </summary>
        /// <param name="window">The window to destroy.</param>
        static void Close(eastl::shared_ptr<Window> window);

        /// <summary>
        /// Called when the window is resized.
        /// </summary>
        Delegate<void(Vector2Int)>& OnSize() { return m_onSize; }

        /// <summary>
        /// Called when the window is moved.
        /// </summary>
        Delegate<void(Vector2Int)>& OnPosition() { return m_onPosition; }

        /// <summary>
        /// Called when the window title changed.
        /// </summary>
        Delegate<void(String)>& OnTitle() { return m_onTitle; }

        /// <summary>
        /// Called when the window has toggled borderless on or off.
        /// </summary>
        Delegate<void(bool)>& OnBorderless() { return m_onBorderless; }

        /// <summary>
        /// Called when the window has toggled resizable on or off.
        /// </summary>
        Delegate<void(bool)>& OnResizable() { return m_onResizable; }

        /// <summary>
        /// Called when the window has toggled floating on or off.
        /// </summary>
        Delegate<void(bool)>& OnFloating() { return m_onFloating; }

        /// <summary>
        /// Called when the has gone fullscreen or windowed.
        /// </summary>
        Delegate<void(bool)>& OnFullscreen() { return m_onFullscreen; }

        /// <summary>
        /// Called when the window requests a close.
        /// </summary>
        Delegate<void()>& OnClose() { return m_onClose; }

        /// <summary>
        /// Called when the window is focused or unfocused.
        /// </summary>
        Delegate<void(bool)>& OnFocus() { return m_onFocus; }

        /// <summary>
        /// Called when the window is minimized or maximized.
        /// </summary>
        Delegate<void(bool)>& OnIconify() { return m_onIconify; }

        /// <summary>
        /// Gets the underlying window object.
        /// </summary>
        const GLFWwindow* GetWindow() const { return m_window; }

        /// <summary>
        /// Gets the underlying surface object.
        /// </summary>
        const Surface* GetSurface() const { return m_surface; }

        /// <summary>
        /// Gets the size of the window in pixels.
        /// </summary>
        /// <param name="checkFullscreen">If in fullscreen and true size will be the screens size.</param>
        const Vector2Int& GetSize(const bool& checkFullscreen = true) const
        {
            return (m_fullscreen && checkFullscreen) ? m_fullscreenSize : m_size;
        }

        /// <summary>
        /// Gets the window title.
        /// </summary>
        const String& GetTitle() const { return m_title; }

        /// <summary>
        /// Sets the window title.
        /// </summary>
        void SetTitle(const String& title);

        /// <summary>
        /// Sets window icon images.
        /// </summary>
        /// <param name="filenames"></param>
        void SetIcons(const eastl::vector<String>& filenames);

        /// <summary>
        /// Sets the window size in pixels.
        /// </summary>
        void SetSize(const Vector2Int& size);

        /// <summary>
        /// Gets the aspect ratio of the window.
        /// </summary>
        const float& GetAspectRatio() const { return m_aspectRatio; }

        /// <summary>
        /// Gets the windows position in pixels.
        /// </summary>
        const Vector2Int& GetPosition() const { return m_position; }

        /// <summary>
        /// Sets the window position to a new position in pixels.
        /// </summary>
        void SetPosition(const Vector2Int& position);

        /// <summary>
        /// Gets whether the window is borderless or not.
        /// </summary>
        const bool& IsBorderless() const { return m_borderless; }

        /// <summary>
        /// Sets if the window is borderless.
        /// </summary>
        void SetBorderless(const bool& borderless);

        /// <summary>
        /// Gets whether the window is resizable or not.
        /// </summary>
        const bool& IsResizable() const { return m_resizable; }

        /// <summary>
        /// Sets if the window is resizable.
        /// </summary>
        void SetResizable(const bool& resizable);

        /// <summary>
        /// Gets whether the window is floating or not.
        /// </summary>
        const bool& IsFloating() const { return m_floating; }

        /// <summary>
        /// Sets if the window is floating. If floating the window will always display above other windows.
        /// </summary>
        void SetFloating(const bool& floating);

        /// <summary>
        /// Gets whether the window is fullscreen or not.
        /// </summary>
        const bool& IsFullscreen() const { return m_fullscreen; }

        /// <summary>
        /// Sets the window to be fullscreen or windowed.
        /// </summary>
        /// <param name="fullscreen">Will the window be fullscreen.</param>
        /// <param name="monitor">The monitor to display the window in.</param>
        void SetFullscreen(const bool& fullscreen, Monitor* monitor = nullptr);

        /// <summary>
        /// Gets if the window is closed.
        /// </summary>
        const bool& IsClosed() const { return m_closed; }

        /// <summary>
        /// Gets if the window is selected.
        /// </summary>
        const bool& IsFocused() const { return m_focused; }

        /// <summary>
        /// Gets the windows is minimized.
        /// </summary>
        const bool& IsIconified() const { return m_iconified; }

        /// <summary>
        /// Sets if the window is minimized.
        /// </summary>
        void SetIconified(const bool& iconify);

        /// <summary>
        /// Swaps the buffers for the window.
        /// </summary>
        void SwapBuffers();

        /// <summary>
        /// Creates the drawing surface.
        /// </summary>
        /// <param name="instance">The vulkan instance.</param>
        /// <param name="allocator">The vulkan allocator/</param>
        /// <param name="surface">The created surface.</param>
        /// <returns>Indicates the success of the operation.</returns>
        VkResult CreateSurface(const VkInstance& instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) const;

    private:
        static bool s_initialized;
        static eastl::vector<eastl::shared_ptr<Window>> s_windows;

        /// <summary>
        /// Gets the window instance associated with the underlying window.
        /// </summary>
        /// <param name="window">The underlying window object.</param>
        /// <returns>The window instance.</returns>
        static eastl::shared_ptr<Window> GetWindow(GLFWwindow* window);

        /// <summary>
        /// Creates a new window.
        /// </summary>
        Window();

        /// <summary>
        /// Closes the window.
        /// </summary>
        void Destroy();

        friend void CallbackError(int error, const char* description);
        friend void CallbackPosition(GLFWwindow* window, int xpos, int ypos);
        friend void CallbackSize(GLFWwindow* window, int width, int height);
        friend void CallbackClose(GLFWwindow* window);
        friend void CallbackFocus(GLFWwindow* window, int focused);
        friend void CallbackIconify(GLFWwindow* window, int iconified);
        friend void CallbackFrame(GLFWwindow* window, int width, int height);

        Vector2Int m_position;
        Vector2Int m_size;
        Vector2Int m_fullscreenSize;
        float m_aspectRatio;

        String m_title;
        bool m_borderless;
        bool m_resizable;
        bool m_floating;
        bool m_fullscreen;
        bool m_closed;
        bool m_focused;
        bool m_iconified;

        GLFWwindow* m_window;
        Surface* m_surface;

        Delegate<void(String)> m_onTitle;
        Delegate<void(Vector2Int)> m_onSize;
        Delegate<void(Vector2Int)> m_onPosition;
        Delegate<void(bool)> m_onBorderless;
        Delegate<void(bool)> m_onResizable;
        Delegate<void(bool)> m_onFloating;
        Delegate<void(bool)> m_onFullscreen;
        Delegate<void()> m_onClose;
        Delegate<void(bool)> m_onFocus;
        Delegate<void(bool)> m_onIconify;
    };
}
