// =============================================================================
// FILE: src/Game/UI/UIManager.h
// PURPOSE: UI rendering and management system
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include <vector>
#include <functional>
#include <queue>

namespace Duality {

// Forward declarations
class VulkanRenderer;
class Window;

// ============================================================================
// UI Element Types
// ============================================================================

enum class UIElementType {
    TEXT,
    IMAGE,
    BUTTON,
    PROGRESS_BAR,
    PANEL,
    DIALOGUE_BOX,
    CROSSHAIR,
    MINIMAP
};

struct UIElement {
    UIElementType type;
    std::string id;
    Rect bounds;
    bool visible = true;
    bool enabled = true;
    f32 opacity = 1.0f;
    i32 zOrder = 0;
    
    // Text element
    std::string text;
    Color textColor = Color::WHITE();
    f32 fontSize = 16.0f;
    bool textCentered = false;
    
    // Image element
    std::string imagePath;
    Color tint = Color::WHITE();
    
    // Button element
    std::function<void()> onClick;
    Color normalColor = Color(100, 100, 100);
    Color hoverColor = Color(150, 150, 150);
    Color pressedColor = Color(80, 80, 80);
    bool isHovered = false;
    bool isPressed = false;
    
    // Progress bar
    f32 progress = 0.0f;
    f32 minValue = 0.0f;
    f32 maxValue = 100.0f;
    Color fillColor = Color::GREEN();
    Color backgroundColor = Color(50, 50, 50);
    bool showPercentage = false;
    
    // Panel
    Color backgroundColor = Color(0, 0, 0, 180);
    bool hasBorder = true;
    Color borderColor = Color::WHITE();
    f32 borderWidth = 1.0f;
    f32 cornerRadius = 0.0f;
};

// ============================================================================
// UI Manager
// ============================================================================

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    void Initialize(VulkanRenderer* renderer, Window* window);
    void Shutdown();
    
    void Update(f32 deltaTime);
    void Render();
    void OnResize(u32 width, u32 height);
    
    // Element management
    UIElement* AddElement(const UIElement& element);
    UIElement* GetElement(const std::string& id);
    void RemoveElement(const std::string& id);
    void ClearElements();
    
    // HUD elements
    void UpdateHealth(f32 current, f32 max);
    void UpdateStamina(f32 current, f32 max);
    void UpdateRadiation(f32 current, f32 max);
    void UpdateObjective(const std::string& objective);
    void UpdateGameTime(f32 seconds);
    void UpdateAmmo(i32 current, i32 max);
    void UpdateCompass(f32 rotation, const std::vector<std::pair<Vec3, std::string>>& markers);
    
    // Dialogue system
    void ShowDialogue(const std::string& text, const std::string& speaker = "Elsy");
    void HideDialogue();
    void AddDialogueChoice(const std::string& text, std::function<void()> callback);
    void ClearDialogueChoices();
    
    // Crosshair
    void SetCrosshairType(const std::string& type);
    void SetCrosshairVisible(bool visible);
    
    // Minimap
    void SetMinimapVisible(bool visible);
    void SetMinimapZoom(f32 zoom);
    void AddMinimapMarker(const Vec3& position, const Color& color, const std::string& icon);
    void ClearMinimapMarkers();
    
    // Notifications
    void ShowNotification(const std::string& message, f32 duration = 3.0f, const Color& color = Color::WHITE());
    void ClearNotifications();
    
    // Debug UI
    void ShowDebugUI(bool show);
    void AddDebugInfo(const std::string& key, const std::string& value);
    void ClearDebugInfo();
    
    // Settings
    void SetUIScale(f32 scale);
    void SetUIOpacity(f32 opacity);
    
private:
    void UpdateLayout();
    void UpdateInput();
    void RenderElement(const UIElement& element);
    void RenderText(const UIElement& element);
    void RenderImage(const UIElement& element);
    void RenderButton(const UIElement& element);
    void RenderProgressBar(const UIElement& element);
    void RenderPanel(const UIElement& element);
    void RenderDialogueBox();
    void RenderCrosshair();
    void RenderMinimap();
    void RenderNotifications();
    void RenderDebugUI();
    
    void UpdateNotificationQueue(f32 deltaTime);
    bool IsPointInRect(const Vec2& point, const Rect& rect);
    
    VulkanRenderer* m_renderer = nullptr;
    Window* m_window = nullptr;
    
    std::unordered_map<std::string, UIElement> m_elements;
    std::vector<UIElement*> m_sortedElements;
    
    // HUD elements
    std::string m_healthBarId;
    std::string m_staminaBarId;
    std::string m_radiationBarId;
    std::string m_objectiveTextId;
    std::string m_timeTextId;
    std::string m_ammoTextId;
    
    // Dialogue system
    struct DialogueState {
        bool visible = false;
        std::string text;
        std::string speaker;
        std::vector<std::pair<std::string, std::function<void()>>> choices;
        f32 typingSpeed = 0.05f;
        f32 typingTimer = 0.0f;
        i32 typedChars = 0;
        std::string displayText;
    } m_dialogue;
    
    // Crosshair
    struct CrosshairState {
        bool visible = true;
        std::string type = "default";
        Vec2 position;
        f32 size = 20.0f;
        Color color = Color::WHITE();
    } m_crosshair;
    
    // Minimap
    struct MinimapState {
        bool visible = true;
        f32 zoom = 1.0f;
        f32 rotation = 0.0f;
        Vec2 position;
        f32 size = 200.0f;
        std::vector<std::pair<Vec3, std::pair<Color, std::string>>> markers;
    } m_minimap;
    
    // Notifications
    struct Notification {
        std::string message;
        Color color;
        f32 duration;
        f32 timer;
    };
    std::vector<Notification> m_notifications;
    
    // Debug UI
    struct DebugState {
        bool visible = false;
        std::unordered_map<std::string, std::string> info;
        f32 fps = 0.0f;
        f32 frameTime = 0.0f;
        f32 gpuTime = 0.0f;
        u32 drawCalls = 0;
        u32 triangleCount = 0;
        u32 memoryUsage = 0;
    } m_debug;
    
    // Settings
    f32 m_uiScale = 1.0f;
    f32 m_globalOpacity = 1.0f;
    Vec2 m_screenSize;
    
    // Input state
    Vec2 m_mousePosition;
    bool m_mouseLeftPressed = false;
    bool m_mouseLeftJustPressed = false;
};

} // namespace Duality