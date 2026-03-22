// =============================================================================
// FILE: src/Engine/Core/Window.h
// PURPOSE: Cross-platform window management with GLFW
// =============================================================================

#pragma once

#include "Types.h"
#include "Engine/Input/InputSystem.h"
#include <functional>
#include <GLFW/glfw3.h>

namespace Duality {

struct WindowConfig {
    std::string title = "Duality: Apocalypse";
    u32 width = 1920;
    u32 height = 1080;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    bool cursorLocked = false;
    bool cursorHidden = false;
    u32 refreshRate = 0; // 0 = default
    u32 samples = 4; // MSAA samples
    bool borderless = false;
    bool maximized = false;
    
    // Window position (if not fullscreen)
    i32 posX = -1; // -1 = center
    i32 posY = -1;
};

class Window {
public:
    Window();
    ~Window();
    
    // Initialize window
    bool Initialize(const WindowConfig& config = WindowConfig());
    void Shutdown();
    
    // Window state
    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;
    void SetShouldClose(bool shouldClose);
    
    // Window properties
    [[nodiscard]] GLFWwindow* GetHandle() const { return m_window; }
    [[nodiscard]] u32 GetWidth() const { return m_width; }
    [[nodiscard]] u32 GetHeight() const { return m_height; }
    [[nodiscard]] f32 GetAspectRatio() const { return static_cast<f32>(m_width) / static_cast<f32>(m_height); }
    [[nodiscard]] bool IsFullscreen() const { return m_fullscreen; }
    [[nodiscard]] bool IsMinimized() const { return m_minimized; }
    [[nodiscard]] bool IsFocused() const { return m_focused; }
    
    // Window manipulation
    void SetTitle(const std::string& title);
    void SetSize(u32 width, u32 height);
    void SetPosition(i32 x, i32 y);
    void SetFullscreen(bool fullscreen);
    void SetVSync(bool enabled);
    void SetCursorMode(CursorMode mode);
    void SetCursorLock(bool locked);
    void SetCursorHidden(bool hidden);
    void Maximize();
    void Minimize();
    void Restore();
    void Focus();
    
    // Input access
    [[nodiscard]] InputSystem* GetInput() { return m_input.get(); }
    
    // Callbacks
    using ResizeCallback = std::function<void(u32 width, u32 height)>;
    using FocusCallback = std::function<void(bool focused)>;
    using CloseCallback = std::function<void()>;
    using DropCallback = std::function<void(const std::vector<std::string>& files)>;
    
    void SetResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }
    void SetFocusCallback(FocusCallback callback) { m_focusCallback = callback; }
    void SetCloseCallback(CloseCallback callback) { m_closeCallback = callback; }
    void SetDropCallback(DropCallback callback) { m_dropCallback = callback; }
    
    // Static callbacks for GLFW
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowFocusCallback(GLFWwindow* window, int focused);
    static void WindowCloseCallback(GLFWwindow* window);
    static void DropCallback(GLFWwindow* window, int count, const char** paths);
    static void ErrorCallback(int error, const char* description);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void CharCallback(GLFWwindow* window, unsigned int codepoint);
    
private:
    void ApplyWindowMode();
    void CenterWindow();
    void UpdateCursorMode();
    static Window* GetWindow(GLFWwindow* window);
    
    GLFWwindow* m_window = nullptr;
    Scope<InputSystem> m_input;
    
    WindowConfig m_config;
    u32 m_width = 0;
    u32 m_height = 0;
    bool m_fullscreen = false;
    bool m_minimized = false;
    bool m_focused = true;
    
    // Callbacks
    ResizeCallback m_resizeCallback;
    FocusCallback m_focusCallback;
    CloseCallback m_closeCallback;
    DropCallback m_dropCallback;
    
    // Static map for callback lookup
    inline static std::unordered_map<GLFWwindow*, Window*> s_windows;
};

} // namespace Duality