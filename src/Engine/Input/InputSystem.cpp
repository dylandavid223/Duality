// =============================================================================
// FILE: src/Engine/Input/InputSystem.cpp
// PURPOSE: Input system implementation
// =============================================================================

#include "InputSystem.h"
#include "Engine/Core/Logger.h"
#include "Engine/Core/Timer.h"

namespace Duality {

void InputSystem::Initialize(GLFWwindow* window) {
    m_window = window;
    
    // Initialize key states
    for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; i++) {
        m_keys[static_cast<KeyCode>(i)] = KeyState{};
    }
    
    // Initialize mouse button states
    for (int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; i++) {
        m_mouseButtons[static_cast<MouseButton>(i)] = KeyState{};
    }
    
    LOG_INFO("Input system initialized");
}

void InputSystem::Shutdown() {
    m_actionBindings.clear();
    m_axisBindings.clear();
    LOG_INFO("Input system shutdown");
}

void InputSystem::Update() {
    // Store previous states
    for (auto& [key, state] : m_keys) {
        state.justPressed = false;
        state.justReleased = false;
    }
    
    for (auto& [button, state] : m_mouseButtons) {
        state.justPressed = false;
        state.justReleased = false;
    }
    
    // Reset mouse delta
    m_mouse.delta = Vec2::ZERO();
    m_mouse.scroll = Vec2::ZERO();
    
    // Update action bindings
    UpdateActionBindings();
    UpdateAxisBindings();
}

bool InputSystem::IsKeyPressed(KeyCode key) const {
    auto it = m_keys.find(key);
    return it != m_keys.end() && it->second.pressed;
}

bool InputSystem::IsKeyJustPressed(KeyCode key) const {
    auto it = m_keys.find(key);
    return it != m_keys.end() && it->second.justPressed;
}

bool InputSystem::IsKeyJustReleased(KeyCode key) const {
    auto it = m_keys.find(key);
    return it != m_keys.end() && it->second.justReleased;
}

f32 InputSystem::GetKeyHoldTime(KeyCode key) const {
    auto it = m_keys.find(key);
    return it != m_keys.end() ? it->second.holdTime : 0.0f;
}

bool InputSystem::IsMouseButtonPressed(MouseButton button) const {
    auto it = m_mouseButtons.find(button);
    return it != m_mouseButtons.end() && it->second.pressed;
}

bool InputSystem::IsMouseButtonJustPressed(MouseButton button) const {
    auto it = m_mouseButtons.find(button);
    return it != m_mouseButtons.end() && it->second.justPressed;
}

bool InputSystem::IsMouseButtonJustReleased(MouseButton button) const {
    auto it = m_mouseButtons.find(button);
    return it != m_mouseButtons.end() && it->second.justReleased;
}

void InputSystem::BindAction(const std::string& name, const ActionBinding& binding) {
    m_actionBindings[name] = binding;
    LOG_DEBUG("Bound action: {}", name);
}

void InputSystem::BindAxis(const std::string& name, const AxisBinding& binding) {
    m_axisBindings[name] = binding;
    LOG_DEBUG("Bound axis: {}", name);
}

void InputSystem::UnbindAction(const std::string& name) {
    m_actionBindings.erase(name);
}

void InputSystem::UnbindAxis(const std::string& name) {
    m_axisBindings.erase(name);
}

f32 InputSystem::GetAxis(const std::string& name) const {
    auto it = m_axisBindings.find(name);
    if (it != m_axisBindings.end()) {
        return it->second.value;
    }
    return 0.0f;
}

void InputSystem::SetCursorMode(CursorMode mode) {
    m_cursorMode = mode;
    
    switch (mode) {
        case CursorMode::Normal:
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case CursorMode::Hidden:
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            break;
        case CursorMode::Locked:
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
    }
}

void InputSystem::SetCursorLocked(bool locked) {
    m_cursorLocked = locked;
    if (locked) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else if (!m_cursorHidden) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void InputSystem::SetCursorHidden(bool hidden) {
    m_cursorHidden = hidden;
    if (!m_cursorLocked) {
        glfwSetInputMode(m_window, GLFW_CURSOR, hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    }
}

void InputSystem::SetCursorPosition(const Vec2& position) {
    glfwSetCursorPos(m_window, static_cast<f64>(position.x), static_cast<f64>(position.y));
    m_mouse.position = position;
}

void InputSystem::HandleKeyEvent(i32 key, i32 scancode, i32 action, i32 mods) {
    KeyCode keyCode = static_cast<KeyCode>(key);
    KeyAction keyAction = static_cast<KeyAction>(action);
    
    UpdateKeyState(keyCode, keyAction);
}

void InputSystem::HandleMouseButtonEvent(i32 button, i32 action, i32 mods) {
    MouseButton mouseButton = static_cast<MouseButton>(button);
    KeyAction keyAction = static_cast<KeyAction>(action);
    
    UpdateKeyState(static_cast<KeyCode>(button), keyAction);
    
    auto& state = m_mouseButtons[mouseButton];
    
    if (action == GLFW_PRESS) {
        state.justPressed = true;
        state.pressed = true;
        state.holdTime = 0.0f;
        state.repeatCount++;
    } else if (action == GLFW_RELEASE) {
        state.justReleased = true;
        state.pressed = false;
        state.repeatCount = 0;
    }
}

void InputSystem::HandleMouseMoveEvent(f32 x, f32 y) {
    Vec2 newPos(x, y);
    
    if (m_firstMouse) {
        m_lastMousePos = newPos;
        m_firstMouse = false;
    }
    
    m_mouse.delta = newPos - m_lastMousePos;
    m_mouse.position = newPos;
    m_lastMousePos = newPos;
}

void InputSystem::HandleScrollEvent(f32 xoffset, f32 yoffset) {
    m_mouse.scroll = Vec2(xoffset, yoffset);
}

void InputSystem::HandleCharEvent(u32 codepoint) {
    // Handle text input (for UI, console, etc.)
}

void InputSystem::UpdateKeyState(KeyCode key, KeyAction action) {
    auto& state = m_keys[key];
    
    if (action == KeyAction::Press) {
        state.justPressed = true;
        state.pressed = true;
        state.holdTime = 0.0f;
        state.repeatCount++;
    } else if (action == KeyAction::Release) {
        state.justReleased = true;
        state.pressed = false;
        state.repeatCount = 0;
    } else if (action == KeyAction::Repeat) {
        state.repeatCount++;
    }
}

void InputSystem::UpdateActionBindings() {
    for (auto& [name, binding] : m_actionBindings) {
        bool pressed = false;
        bool released = false;
        bool repeated = false;
        
        for (KeyCode key : binding.keys) {
            if (IsKeyJustPressed(key)) {
                pressed = true;
            }
            if (IsKeyJustReleased(key)) {
                released = true;
            }
            if (IsKeyPressed(key) && GetKeyHoldTime(key) > 0.2f && GetKeyHoldTime(key) - 0.2f < m_deltaTime) {
                repeated = true;
            }
        }
        
        if (pressed && binding.onPress) {
            binding.onPress();
        }
        if (released && binding.onRelease) {
            binding.onRelease();
        }
        if (repeated && binding.onRepeat) {
            binding.onRepeat();
        }
    }
}

void InputSystem::UpdateAxisBindings() {
    for (auto& [name, binding] : m_axisBindings) {
        f32 value = 0.0f;
        
        for (const auto& [key, multiplier] : binding.keys) {
            if (IsKeyPressed(key)) {
                value += multiplier;
            }
        }
        
        value *= binding.sensitivity;
        
        // Apply deadzone
        if (std::abs(value) < binding.deadzone) {
            value = 0.0f;
        }
        
        // Smoothing (optional)
        value = Math::Lerp(binding.value, value, 0.2f);
        
        binding.value = value;
        
        if (binding.callback) {
            binding.callback(value);
        }
    }
}

void InputSystem::ResetFrameStates() {
    // Reset per-frame states
    for (auto& [key, state] : m_keys) {
        state.justPressed = false;
        state.justReleased = false;
    }
    
    for (auto& [button, state] : m_mouseButtons) {
        state.justPressed = false;
        state.justReleased = false;
    }
    
    m_mouse.delta = Vec2::ZERO();
    m_mouse.scroll = Vec2::ZERO();
}

} // namespace Duality