// =============================================================================
// FILE: src/Engine/Input/InputSystem.h
// PURPOSE: Input handling system with key bindings and action mapping
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>
#include <vector>

namespace Duality {

// ============================================================================
// Input Enums
// ============================================================================

enum class KeyCode {
    // Letters
    Space = GLFW_KEY_SPACE,
    Apostrophe = GLFW_KEY_APOSTROPHE,
    Comma = GLFW_KEY_COMMA,
    Minus = GLFW_KEY_MINUS,
    Period = GLFW_KEY_PERIOD,
    Slash = GLFW_KEY_SLASH,
    _0 = GLFW_KEY_0,
    _1 = GLFW_KEY_1,
    _2 = GLFW_KEY_2,
    _3 = GLFW_KEY_3,
    _4 = GLFW_KEY_4,
    _5 = GLFW_KEY_5,
    _6 = GLFW_KEY_6,
    _7 = GLFW_KEY_7,
    _8 = GLFW_KEY_8,
    _9 = GLFW_KEY_9,
    Semicolon = GLFW_KEY_SEMICOLON,
    Equal = GLFW_KEY_EQUAL,
    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,
    LeftBracket = GLFW_KEY_LEFT_BRACKET,
    Backslash = GLFW_KEY_BACKSLASH,
    RightBracket = GLFW_KEY_RIGHT_BRACKET,
    GraveAccent = GLFW_KEY_GRAVE_ACCENT,
    
    // Function keys
    Escape = GLFW_KEY_ESCAPE,
    Enter = GLFW_KEY_ENTER,
    Tab = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE,
    Insert = GLFW_KEY_INSERT,
    Delete = GLFW_KEY_DELETE,
    Right = GLFW_KEY_RIGHT,
    Left = GLFW_KEY_LEFT,
    Down = GLFW_KEY_DOWN,
    Up = GLFW_KEY_UP,
    PageUp = GLFW_KEY_PAGE_UP,
    PageDown = GLFW_KEY_PAGE_DOWN,
    Home = GLFW_KEY_HOME,
    End = GLFW_KEY_END,
    CapsLock = GLFW_KEY_CAPS_LOCK,
    ScrollLock = GLFW_KEY_SCROLL_LOCK,
    NumLock = GLFW_KEY_NUM_LOCK,
    PrintScreen = GLFW_KEY_PRINT_SCREEN,
    Pause = GLFW_KEY_PAUSE,
    
    // F-keys
    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,
    F13 = GLFW_KEY_F13,
    F14 = GLFW_KEY_F14,
    F15 = GLFW_KEY_F15,
    F16 = GLFW_KEY_F16,
    F17 = GLFW_KEY_F17,
    F18 = GLFW_KEY_F18,
    F19 = GLFW_KEY_F19,
    F20 = GLFW_KEY_F20,
    F21 = GLFW_KEY_F21,
    F22 = GLFW_KEY_F22,
    F23 = GLFW_KEY_F23,
    F24 = GLFW_KEY_F24,
    F25 = GLFW_KEY_F25,
    
    // Keypad
    KP_0 = GLFW_KEY_KP_0,
    KP_1 = GLFW_KEY_KP_1,
    KP_2 = GLFW_KEY_KP_2,
    KP_3 = GLFW_KEY_KP_3,
    KP_4 = GLFW_KEY_KP_4,
    KP_5 = GLFW_KEY_KP_5,
    KP_6 = GLFW_KEY_KP_6,
    KP_7 = GLFW_KEY_KP_7,
    KP_8 = GLFW_KEY_KP_8,
    KP_9 = GLFW_KEY_KP_9,
    KP_Decimal = GLFW_KEY_KP_DECIMAL,
    KP_Divide = GLFW_KEY_KP_DIVIDE,
    KP_Multiply = GLFW_KEY_KP_MULTIPLY,
    KP_Subtract = GLFW_KEY_KP_SUBTRACT,
    KP_Add = GLFW_KEY_KP_ADD,
    KP_Enter = GLFW_KEY_KP_ENTER,
    KP_Equal = GLFW_KEY_KP_EQUAL,
    
    // Modifiers
    LeftShift = GLFW_KEY_LEFT_SHIFT,
    LeftControl = GLFW_KEY_LEFT_CONTROL,
    LeftAlt = GLFW_KEY_LEFT_ALT,
    LeftSuper = GLFW_KEY_LEFT_SUPER,
    RightShift = GLFW_KEY_RIGHT_SHIFT,
    RightControl = GLFW_KEY_RIGHT_CONTROL,
    RightAlt = GLFW_KEY_RIGHT_ALT,
    RightSuper = GLFW_KEY_RIGHT_SUPER,
    
    // Mouse buttons
    MouseLeft = GLFW_MOUSE_BUTTON_1,
    MouseRight = GLFW_MOUSE_BUTTON_2,
    MouseMiddle = GLFW_MOUSE_BUTTON_3,
    MouseButton4 = GLFW_MOUSE_BUTTON_4,
    MouseButton5 = GLFW_MOUSE_BUTTON_5,
    MouseButton6 = GLFW_MOUSE_BUTTON_6,
    MouseButton7 = GLFW_MOUSE_BUTTON_7,
    MouseButton8 = GLFW_MOUSE_BUTTON_8,
    
    Unknown = GLFW_KEY_UNKNOWN
};

enum class KeyAction {
    Press = GLFW_PRESS,
    Release = GLFW_RELEASE,
    Repeat = GLFW_REPEAT
};

enum class MouseButton {
    Left = GLFW_MOUSE_BUTTON_1,
    Right = GLFW_MOUSE_BUTTON_2,
    Middle = GLFW_MOUSE_BUTTON_3,
    Button4 = GLFW_MOUSE_BUTTON_4,
    Button5 = GLFW_MOUSE_BUTTON_5,
    Button6 = GLFW_MOUSE_BUTTON_6,
    Button7 = GLFW_MOUSE_BUTTON_7,
    Button8 = GLFW_MOUSE_BUTTON_8
};

enum class CursorMode {
    Normal,
    Hidden,
    Locked
};

// ============================================================================
// Input State
// ============================================================================

struct KeyState {
    bool pressed = false;
    bool justPressed = false;
    bool justReleased = false;
    f32 holdTime = 0.0f;
    u32 repeatCount = 0;
};

struct MouseState {
    Vec2 position = Vec2::ZERO();
    Vec2 delta = Vec2::ZERO();
    Vec2 scroll = Vec2::ZERO();
    std::unordered_map<MouseButton, KeyState> buttons;
};

// ============================================================================
// Action Mapping
// ============================================================================

using ActionCallback = std::function<void()>;
using AxisCallback = std::function<void(f32 value)>;

struct ActionBinding {
    std::vector<KeyCode> keys;
    ActionCallback onPress;
    ActionCallback onRelease;
    ActionCallback onRepeat;
};

struct AxisBinding {
    std::vector<std::pair<KeyCode, f32>> keys; // key and value (+1 or -1)
    AxisCallback callback;
    f32 value = 0.0f;
    f32 sensitivity = 1.0f;
    f32 deadzone = 0.1f;
};

// ============================================================================
// Input System
// ============================================================================

class InputSystem {
public:
    InputSystem() = default;
    ~InputSystem() = default;
    
    void Initialize(GLFWwindow* window);
    void Shutdown();
    void Update();
    
    // Raw input state
    [[nodiscard]] bool IsKeyPressed(KeyCode key) const;
    [[nodiscard]] bool IsKeyJustPressed(KeyCode key) const;
    [[nodiscard]] bool IsKeyJustReleased(KeyCode key) const;
    [[nodiscard]] f32 GetKeyHoldTime(KeyCode key) const;
    
    [[nodiscard]] bool IsMouseButtonPressed(MouseButton button) const;
    [[nodiscard]] bool IsMouseButtonJustPressed(MouseButton button) const;
    [[nodiscard]] bool IsMouseButtonJustReleased(MouseButton button) const;
    
    [[nodiscard]] Vec2 GetMousePosition() const { return m_mouse.position; }
    [[nodiscard]] Vec2 GetMouseDelta() const { return m_mouse.delta; }
    [[nodiscard]] Vec2 GetMouseScroll() const { return m_mouse.scroll; }
    
    // Action mapping
    void BindAction(const std::string& name, const ActionBinding& binding);
    void BindAxis(const std::string& name, const AxisBinding& binding);
    void UnbindAction(const std::string& name);
    void UnbindAxis(const std::string& name);
    
    [[nodiscard]] f32 GetAxis(const std::string& name) const;
    
    // Cursor control
    void SetCursorMode(CursorMode mode);
    void SetCursorLocked(bool locked);
    void SetCursorHidden(bool hidden);
    void SetCursorPosition(const Vec2& position);
    
    // Event handlers (for GLFW callbacks)
    void HandleKeyEvent(i32 key, i32 scancode, i32 action, i32 mods);
    void HandleMouseButtonEvent(i32 button, i32 action, i32 mods);
    void HandleMouseMoveEvent(f32 x, f32 y);
    void HandleScrollEvent(f32 xoffset, f32 yoffset);
    void HandleCharEvent(u32 codepoint);
    
private:
    void UpdateKeyState(KeyCode key, KeyAction action);
    void UpdateActionBindings();
    void UpdateAxisBindings();
    void ResetFrameStates();
    
    GLFWwindow* m_window = nullptr;
    
    // Key states
    std::unordered_map<KeyCode, KeyState> m_keys;
    std::unordered_map<MouseButton, KeyState> m_mouseButtons;
    
    // Mouse state
    MouseState m_mouse;
    Vec2 m_lastMousePos = Vec2::ZERO();
    bool m_firstMouse = true;
    
    // Action mappings
    std::unordered_map<std::string, ActionBinding> m_actionBindings;
    std::unordered_map<std::string, AxisBinding> m_axisBindings;
    
    // Cursor state
    CursorMode m_cursorMode = CursorMode::Normal;
    bool m_cursorLocked = false;
    bool m_cursorHidden = false;
    
    // Delta time for hold times
    f32 m_deltaTime = 0.0f;
};

} // namespace Duality