// =============================================================================
// FILE: src/Engine/Core/Window.cpp
// PURPOSE: GLFW window implementation
// =============================================================================

#include "Window.h"
#include "Logger.h"
#include <GLFW/glfw3.h>

namespace Duality {

Window::Window() = default;

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(const WindowConfig& config) {
    m_config = config;
    m_width = config.width;
    m_height = config.height;
    m_fullscreen = config.fullscreen;
    
    LOG_INFO("Initializing window: {} x {}", config.width, config.height);
    
    // Initialize GLFW
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    
    // Set GLFW error callback
    glfwSetErrorCallback(ErrorCallback);
    
    // Configure GLFW
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Vulkan
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_REFRESH_RATE, config.refreshRate);
    glfwWindowHint(GLFW_SAMPLES, config.samples);
    glfwWindowHint(GLFW_DECORATED, !config.borderless);
    glfwWindowHint(GLFW_MAXIMIZED, config.maximized ? GLFW_TRUE : GLFW_FALSE);
    
    // Create window
    if (config.fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        if (config.borderless) {
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            m_window = glfwCreateWindow(mode->width, mode->height, config.title.c_str(), nullptr, nullptr);
        } else {
            m_window = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);
        }
    } else {
        m_window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    }
    
    if (!m_window) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    
    // Store window pointer for callbacks
    s_windows[m_window] = this;
    
    // Set user pointer
    glfwSetWindowUserPointer(m_window, this);
    
    // Set callbacks
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetWindowFocusCallback(m_window, WindowFocusCallback);
    glfwSetWindowCloseCallback(m_window, WindowCloseCallback);
    glfwSetDropCallback(m_window, DropCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);
    glfwSetCharCallback(m_window, CharCallback);
    
    // Center window if position not specified
    if (config.posX == -1 || config.posY == -1) {
        CenterWindow();
    } else {
        glfwSetWindowPos(m_window, config.posX, config.posY);
    }
    
    // Create input system
    m_input = MakeScope<InputSystem>();
    m_input->Initialize(m_window);
    
    // Set cursor mode
    SetCursorLock(config.cursorLocked);
    SetCursorHidden(config.cursorHidden);
    
    // Set VSync
    SetVSync(config.vsync);
    
    // Get actual framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    m_width = static_cast<u32>(fbWidth);
    m_height = static_cast<u32>(fbHeight);
    
    LOG_INFO("Window created successfully: {}x{}", m_width, m_height);
    
    return true;
}

void Window::Shutdown() {
    if (m_window) {
        s_windows.erase(m_window);
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    
    glfwTerminate();
    
    LOG_INFO("Window shutdown complete");
}

void Window::PollEvents() {
    glfwPollEvents();
    if (m_input) {
        m_input->Update();
    }
    
    // Check if window is minimized
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    m_minimized = (width == 0 || height == 0);
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_window);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::SetShouldClose(bool shouldClose) {
    glfwSetWindowShouldClose(m_window, shouldClose);
}

void Window::SetTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}

void Window::SetSize(u32 width, u32 height) {
    glfwSetWindowSize(m_window, static_cast<i32>(width), static_cast<i32>(height));
    m_width = width;
    m_height = height;
}

void Window::SetPosition(i32 x, i32 y) {
    glfwSetWindowPos(m_window, x, y);
}

void Window::SetFullscreen(bool fullscreen) {
    if (fullscreen == m_fullscreen) return;
    
    if (fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(m_window, nullptr, 
                             m_config.posX, m_config.posY, 
                             m_config.width, m_config.height, 
                             m_config.refreshRate);
    }
    
    m_fullscreen = fullscreen;
}

void Window::SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    m_config.vsync = enabled;
}

void Window::SetCursorMode(CursorMode mode) {
    if (m_input) {
        m_input->SetCursorMode(mode);
    }
}

void Window::SetCursorLock(bool locked) {
    if (m_input) {
        m_input->SetCursorLocked(locked);
    }
    m_config.cursorLocked = locked;
}

void Window::SetCursorHidden(bool hidden) {
    if (m_input) {
        m_input->SetCursorHidden(hidden);
    }
    m_config.cursorHidden = hidden;
}

void Window::Maximize() {
    glfwMaximizeWindow(m_window);
}

void Window::Minimize() {
    glfwIconifyWindow(m_window);
}

void Window::Restore() {
    glfwRestoreWindow(m_window);
}

void Window::Focus() {
    glfwFocusWindow(m_window);
}

void Window::ApplyWindowMode() {
    // Implementation for applying window mode changes
}

void Window::CenterWindow() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    i32 x = (mode->width - static_cast<i32>(m_width)) / 2;
    i32 y = (mode->height - static_cast<i32>(m_height)) / 2;
    
    glfwSetWindowPos(m_window, x, y);
}

void Window::UpdateCursorMode() {
    if (m_config.cursorLocked) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (m_config.cursorHidden) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

Window* Window::GetWindow(GLFWwindow* window) {
    auto it = s_windows.find(window);
    return it != s_windows.end() ? it->second : nullptr;
}

// ============================================================================
// Static Callbacks
// ============================================================================

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = GetWindow(window);
    if (win) {
        win->m_width = static_cast<u32>(width);
        win->m_height = static_cast<u32>(height);
        
        if (win->m_resizeCallback) {
            win->m_resizeCallback(static_cast<u32>(width), static_cast<u32>(height));
        }
    }
}

void Window::WindowFocusCallback(GLFWwindow* window, int focused) {
    Window* win = GetWindow(window);
    if (win) {
        win->m_focused = focused != 0;
        if (win->m_focusCallback) {
            win->m_focusCallback(win->m_focused);
        }
    }
}

void Window::WindowCloseCallback(GLFWwindow* window) {
    Window* win = GetWindow(window);
    if (win && win->m_closeCallback) {
        win->m_closeCallback();
    }
}

void Window::DropCallback(GLFWwindow* window, int count, const char** paths) {
    Window* win = GetWindow(window);
    if (win && win->m_dropCallback) {
        std::vector<std::string> files;
        for (int i = 0; i < count; i++) {
            files.emplace_back(paths[i]);
        }
        win->m_dropCallback(files);
    }
}

void Window::ErrorCallback(int error, const char* description) {
    LOG_ERROR("GLFW Error {}: {}", error, description);
}

void Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* win = GetWindow(window);
    if (win && win->m_input) {
        win->m_input->HandleKeyEvent(key, scancode, action, mods);
    }
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* win = GetWindow(window);
    if (win && win->m_input) {
        win->m_input->HandleMouseButtonEvent(button, action, mods);
    }
}

void Window::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Window* win = GetWindow(window);
    if (win && win->m_input) {
        win->m_input->HandleMouseMoveEvent(static_cast<f32>(xpos), static_cast<f32>(ypos));
    }
}

void Window::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Window* win = GetWindow(window);
    if (win && win->m_input) {
        win->m_input->HandleScrollEvent(static_cast<f32>(xoffset), static_cast<f32>(yoffset));
    }
}

void Window::CharCallback(GLFWwindow* window, unsigned int codepoint) {
    Window* win = GetWindow(window);
    if (win && win->m_input) {
        win->m_input->HandleCharEvent(codepoint);
    }
}

} // namespace Duality