// =============================================================================
// FILE: src/Game/UI/UIManager.cpp
// PURPOSE: UI Manager implementation
// =============================================================================

#include "UIManager.h"
#include "Engine/Core/Logger.h"
#include "Engine/Renderer/VulkanRenderer.h"
#include "Engine/Core/Window.h"
#include <iomanip>
#include <sstream>

namespace Duality {

UIManager::UIManager() {
    LOG_INFO("UIManager created");
}

UIManager::~UIManager() {
    Shutdown();
}

void UIManager::Initialize(VulkanRenderer* renderer, Window* window) {
    m_renderer = renderer;
    m_window = window;
    
    if (m_window) {
        m_screenSize = Vec2(static_cast<f32>(m_window->GetWidth()), static_cast<f32>(m_window->GetHeight()));
    }
    
    // Create default HUD elements
    // Health bar
    UIElement healthBar;
    healthBar.type = UIElementType::PROGRESS_BAR;
    healthBar.id = "HealthBar";
    healthBar.bounds = Rect(20, 20, 300, 30);
    healthBar.fillColor = Color::RED();
    healthBar.backgroundColor = Color(50, 50, 50);
    healthBar.showPercentage = true;
    m_healthBarId = healthBar.id;
    AddElement(healthBar);
    
    // Stamina bar
    UIElement staminaBar;
    staminaBar.type = UIElementType::PROGRESS_BAR;
    staminaBar.id = "StaminaBar";
    staminaBar.bounds = Rect(20, 60, 300, 20);
    staminaBar.fillColor = Color::GREEN();
    staminaBar.backgroundColor = Color(50, 50, 50);
    m_staminaBarId = staminaBar.id;
    AddElement(staminaBar);
    
    // Radiation bar
    UIElement radiationBar;
    radiationBar.type = UIElementType::PROGRESS_BAR;
    radiationBar.id = "RadiationBar";
    radiationBar.bounds = Rect(20, 90, 300, 20);
    radiationBar.fillColor = Color(100, 255, 100);
    radiationBar.backgroundColor = Color(50, 50, 50);
    m_radiationBarId = radiationBar.id;
    AddElement(radiationBar);
    
    // Objective text
    UIElement objectiveText;
    objectiveText.type = UIElementType::TEXT;
    objectiveText.id = "ObjectiveText";
    objectiveText.bounds = Rect(0, m_screenSize.y - 60, m_screenSize.x, 40);
    objectiveText.text = "Survive. Find a way off Earth.";
    objectiveText.textColor = Color::WHITE();
    objectiveText.fontSize = 18.0f;
    objectiveText.textCentered = true;
    m_objectiveTextId = objectiveText.id;
    AddElement(objectiveText);
    
    // Time text
    UIElement timeText;
    timeText.type = UIElementType::TEXT;
    timeText.id = "TimeText";
    timeText.bounds = Rect(m_screenSize.x - 150, 20, 130, 30);
    timeText.text = "00:00:00";
    timeText.textColor = Color::WHITE();
    timeText.fontSize = 14.0f;
    m_timeTextId = timeText.id;
    AddElement(timeText);
    
    // Ammo text
    UIElement ammoText;
    ammoText.type = UIElementType::TEXT;
    ammoText.id = "AmmoText";
    ammoText.bounds = Rect(m_screenSize.x - 150, m_screenSize.y - 40, 130, 30);
    ammoText.text = "0 / 0";
    ammoText.textColor = Color::WHITE();
    ammoText.fontSize = 16.0f;
    ammoText.textCentered = true;
    m_ammoTextId = ammoText.id;
    AddElement(ammoText);
    
    LOG_INFO("UIManager initialized");
}

void UIManager::Shutdown() {
    ClearElements();
    ClearDialogueChoices();
    ClearNotifications();
    ClearMinimapMarkers();
    ClearDebugInfo();
    LOG_INFO("UIManager shutdown");
}

void UIManager::Update(f32 deltaTime) {
    // Update mouse position
    if (m_window && m_window->GetInput()) {
        m_mousePosition = m_window->GetInput()->GetMousePosition();
        m_mouseLeftPressed = m_window->GetInput()->IsMouseButtonPressed(MouseButton::Left);
        m_mouseLeftJustPressed = m_window->GetInput()->IsMouseButtonJustPressed(MouseButton::Left);
    }
    
    // Update input for UI elements
    UpdateInput();
    
    // Update notifications
    UpdateNotificationQueue(deltaTime);
    
    // Update dialogue typing
    if (m_dialogue.visible && m_dialogue.typingTimer > 0.0f) {
        m_dialogue.typingTimer -= deltaTime;
        if (m_dialogue.typingTimer <= 0.0f) {
            if (m_dialogue.typedChars < static_cast<i32>(m_dialogue.text.length())) {
                m_dialogue.displayText += m_dialogue.text[m_dialogue.typedChars];
                m_dialogue.typedChars++;
                m_dialogue.typingTimer = m_dialogue.typingSpeed;
            }
        }
    }
    
    // Update debug info
    if (m_debug.visible) {
        // Get performance metrics from renderer
        // TODO: Implement renderer stats
    }
}

void UIManager::Render() {
    if (!m_renderer) return;
    
    // Sort elements by z-order
    m_sortedElements.clear();
    for (auto& [id, element] : m_elements) {
        if (element.visible) {
            m_sortedElements.push_back(&element);
        }
    }
    std::sort(m_sortedElements.begin(), m_sortedElements.end(),
        [](const UIElement* a, const UIElement* b) {
            return a->zOrder < b->zOrder;
        });
    
    // Render all elements
    for (auto* element : m_sortedElements) {
        RenderElement(*element);
    }
    
    // Render overlay elements
    RenderCrosshair();
    RenderMinimap();
    RenderDialogueBox();
    RenderNotifications();
    RenderDebugUI();
}

void UIManager::OnResize(u32 width, u32 height) {
    m_screenSize = Vec2(static_cast<f32>(width), static_cast<f32>(height));
    
    // Update layout
    UpdateLayout();
}

UIElement* UIManager::AddElement(const UIElement& element) {
    auto [it, inserted] = m_elements.emplace(element.id, element);
    if (inserted) {
        LOG_DEBUG("Added UI element: {}", element.id);
        return &it->second;
    }
    return nullptr;
}

UIElement* UIManager::GetElement(const std::string& id) {
    auto it = m_elements.find(id);
    return it != m_elements.end() ? &it->second : nullptr;
}

void UIManager::RemoveElement(const std::string& id) {
    auto it = m_elements.find(id);
    if (it != m_elements.end()) {
        m_elements.erase(it);
        LOG_DEBUG("Removed UI element: {}", id);
    }
}

void UIManager::ClearElements() {
    m_elements.clear();
    m_sortedElements.clear();
}

void UIManager::UpdateHealth(f32 current, f32 max) {
    auto* element = GetElement(m_healthBarId);
    if (element) {
        element->progress = current / max;
        if (element->showPercentage) {
            element->text = std::format("{:.0f}%", element->progress * 100.0f);
        }
        
        // Change color based on health
        if (element->progress > 0.6f) {
            element->fillColor = Color::GREEN();
        } else if (element->progress > 0.3f) {
            element->fillColor = Color::YELLOW();
        } else {
            element->fillColor = Color::RED();
        }
    }
}

void UIManager::UpdateStamina(f32 current, f32 max) {
    auto* element = GetElement(m_staminaBarId);
    if (element) {
        element->progress = current / max;
        if (element->progress < 0.2f) {
            element->fillColor = Color::RED();
        } else if (element->progress < 0.5f) {
            element->fillColor = Color::YELLOW();
        } else {
            element->fillColor = Color::GREEN();
        }
    }
}

void UIManager::UpdateRadiation(f32 current, f32 max) {
    auto* element = GetElement(m_radiationBarId);
    if (element) {
        element->progress = current / max;
        
        // Change color based on radiation level
        if (element->progress > 0.7f) {
            element->fillColor = Color::RED();
        } else if (element->progress > 0.3f) {
            element->fillColor = Color::ORANGE();
        } else {
            element->fillColor = Color::GREEN();
        }
    }
}

void UIManager::UpdateObjective(const std::string& objective) {
    auto* element = GetElement(m_objectiveTextId);
    if (element) {
        element->text = objective;
    }
}

void UIManager::UpdateGameTime(f32 seconds) {
    auto* element = GetElement(m_timeTextId);
    if (element) {
        i32 hours = static_cast<i32>(seconds) / 3600;
        i32 minutes = (static_cast<i32>(seconds) % 3600) / 60;
        i32 secs = static_cast<i32>(seconds) % 60;
        element->text = std::format("{:02d}:{:02d}:{:02d}", hours, minutes, secs);
    }
}

void UIManager::UpdateAmmo(i32 current, i32 max) {
    auto* element = GetElement(m_ammoTextId);
    if (element) {
        element->text = std::format("{} / {}", current, max);
        
        // Change color if low ammo
        if (current == 0) {
            element->textColor = Color::RED();
        } else if (current < max * 0.2f) {
            element->textColor = Color::YELLOW();
        } else {
            element->textColor = Color::WHITE();
        }
    }
}

void UIManager::UpdateCompass(f32 rotation, const std::vector<std::pair<Vec3, std::string>>& markers) {
    // TODO: Implement compass rendering
}

void UIManager::ShowDialogue(const std::string& text, const std::string& speaker) {
    m_dialogue.visible = true;
    m_dialogue.text = text;
    m_dialogue.speaker = speaker;
    m_dialogue.displayText = "";
    m_dialogue.typedChars = 0;
    m_dialogue.typingTimer = m_dialogue.typingSpeed;
}

void UIManager::HideDialogue() {
    m_dialogue.visible = false;
    m_dialogue.displayText = "";
    m_dialogue.choices.clear();
}

void UIManager::AddDialogueChoice(const std::string& text, std::function<void()> callback) {
    m_dialogue.choices.emplace_back(text, callback);
}

void UIManager::ClearDialogueChoices() {
    m_dialogue.choices.clear();
}

void UIManager::SetCrosshairType(const std::string& type) {
    m_crosshair.type = type;
}

void UIManager::SetCrosshairVisible(bool visible) {
    m_crosshair.visible = visible;
}

void UIManager::SetMinimapVisible(bool visible) {
    m_minimap.visible = visible;
}

void UIManager::SetMinimapZoom(f32 zoom) {
    m_minimap.zoom = std::clamp(zoom, 0.5f, 4.0f);
}

void UIManager::AddMinimapMarker(const Vec3& position, const Color& color, const std::string& icon) {
    m_minimap.markers.emplace_back(position, std::make_pair(color, icon));
}

void UIManager::ClearMinimapMarkers() {
    m_minimap.markers.clear();
}

void UIManager::ShowNotification(const std::string& message, f32 duration, const Color& color) {
    m_notifications.push_back({message, color, duration, duration});
}

void UIManager::ClearNotifications() {
    m_notifications.clear();
}

void UIManager::ShowDebugUI(bool show) {
    m_debug.visible = show;
}

void UIManager::AddDebugInfo(const std::string& key, const std::string& value) {
    m_debug.info[key] = value;
}

void UIManager::ClearDebugInfo() {
    m_debug.info.clear();
}

void UIManager::SetUIScale(f32 scale) {
    m_uiScale = std::clamp(scale, 0.5f, 2.0f);
    UpdateLayout();
}

void UIManager::SetUIOpacity(f32 opacity) {
    m_globalOpacity = std::clamp(opacity, 0.0f, 1.0f);
}

void UIManager::UpdateLayout() {
    // Update positions based on screen size and UI scale
    auto* healthBar = GetElement(m_healthBarId);
    if (healthBar) {
        healthBar->bounds = Rect(20 * m_uiScale, 20 * m_uiScale, 300 * m_uiScale, 30 * m_uiScale);
    }
    
    auto* staminaBar = GetElement(m_staminaBarId);
    if (staminaBar) {
        staminaBar->bounds = Rect(20 * m_uiScale, 60 * m_uiScale, 300 * m_uiScale, 20 * m_uiScale);
    }
    
    auto* radiationBar = GetElement(m_radiationBarId);
    if (radiationBar) {
        radiationBar->bounds = Rect(20 * m_uiScale, 90 * m_uiScale, 300 * m_uiScale, 20 * m_uiScale);
    }
    
    auto* objectiveText = GetElement(m_objectiveTextId);
    if (objectiveText) {
        objectiveText->bounds = Rect(0, m_screenSize.y - 60 * m_uiScale, m_screenSize.x, 40 * m_uiScale);
        objectiveText->fontSize = 18.0f * m_uiScale;
    }
    
    auto* timeText = GetElement(m_timeTextId);
    if (timeText) {
        timeText->bounds = Rect(m_screenSize.x - 150 * m_uiScale, 20 * m_uiScale, 130 * m_uiScale, 30 * m_uiScale);
        timeText->fontSize = 14.0f * m_uiScale;
    }
    
    auto* ammoText = GetElement(m_ammoTextId);
    if (ammoText) {
        ammoText->bounds = Rect(m_screenSize.x - 150 * m_uiScale, m_screenSize.y - 40 * m_uiScale, 130 * m_uiScale, 30 * m_uiScale);
        ammoText->fontSize = 16.0f * m_uiScale;
    }
    
    // Update crosshair position
    m_crosshair.position = Vec2(m_screenSize.x * 0.5f, m_screenSize.y * 0.5f);
    m_crosshair.size = 20.0f * m_uiScale;
    
    // Update minimap position
    m_minimap.position = Vec2(m_screenSize.x - 220 * m_uiScale, 20 * m_uiScale);
    m_minimap.size = 200.0f * m_uiScale;
}

void UIManager::UpdateInput() {
    // Check button clicks
    for (auto& [id, element] : m_elements) {
        if (element.type == UIElementType::BUTTON && element.enabled && element.visible) {
            bool wasHovered = element.isHovered;
            element.isHovered = IsPointInRect(m_mousePosition, element.bounds);
            
            if (element.isHovered) {
                if (m_mouseLeftJustPressed) {
                    element.isPressed = true;
                } else if (!m_mouseLeftPressed && element.isPressed) {
                    element.isPressed = false;
                    if (element.onClick) {
                        element.onClick();
                    }
                }
            } else {
                element.isPressed = false;
            }
        }
    }
    
    // Handle dialogue choices
    if (m_dialogue.visible && !m_dialogue.choices.empty()) {
        // TODO: Render and handle choice selection
    }
}

void UIManager::RenderElement(const UIElement& element) {
    switch (element.type) {
        case UIElementType::TEXT:
            RenderText(element);
            break;
        case UIElementType::IMAGE:
            RenderImage(element);
            break;
        case UIElementType::BUTTON:
            RenderButton(element);
            break;
        case UIElementType::PROGRESS_BAR:
            RenderProgressBar(element);
            break;
        case UIElementType::PANEL:
            RenderPanel(element);
            break;
        default:
            break;
    }
}

void UIManager::RenderText(const UIElement& element) {
    // TODO: Implement text rendering with font atlas
    // This would use the Vulkan renderer to draw text
    LOG_TRACE("Rendering text: {}", element.text);
}

void UIManager::RenderImage(const UIElement& element) {
    // TODO: Implement image rendering
    LOG_TRACE("Rendering image: {}", element.imagePath);
}

void UIManager::RenderButton(const UIElement& element) {
    // Determine color based on state
    Color color = element.normalColor;
    if (element.isPressed) {
        color = element.pressedColor;
    } else if (element.isHovered) {
        color = element.hoverColor;
    }
    
    // Render background
    UIElement bg;
    bg.type = UIElementType::PANEL;
    bg.bounds = element.bounds;
    bg.backgroundColor = color;
    bg.hasBorder = true;
    bg.borderColor = element.textColor;
    bg.cornerRadius = 5.0f;
    RenderPanel(bg);
    
    // Render text
    UIElement text;
    text.type = UIElementType::TEXT;
    text.bounds = element.bounds;
    text.text = element.text;
    text.textColor = element.textColor;
    text.fontSize = element.fontSize;
    text.textCentered = true;
    RenderText(text);
}

void UIManager::RenderProgressBar(const UIElement& element) {
    // Render background
    UIElement bg;
    bg.type = UIElementType::PANEL;
    bg.bounds = element.bounds;
    bg.backgroundColor = element.backgroundColor;
    bg.hasBorder = true;
    bg.borderColor = Color::WHITE();
    bg.cornerRadius = 3.0f;
    RenderPanel(bg);
    
    // Render fill
    UIElement fill;
    fill.type = UIElementType::PANEL;
    fill.bounds = Rect(
        element.bounds.x,
        element.bounds.y,
        element.bounds.width * element.progress,
        element.bounds.height
    );
    fill.backgroundColor = element.fillColor;
    fill.hasBorder = false;
    fill.cornerRadius = 3.0f;
    RenderPanel(fill);
    
    // Render percentage text
    if (element.showPercentage) {
        UIElement text;
        text.type = UIElementType::TEXT;
        text.bounds = element.bounds;
        text.text = element.text;
        text.textColor = Color::WHITE();
        text.fontSize = element.fontSize * 0.8f;
        text.textCentered = true;
        RenderText(text);
    }
}

void UIManager::RenderPanel(const UIElement& element) {
    // TODO: Implement panel rendering with rounded corners
    // This would use the Vulkan renderer to draw a rectangle
    LOG_TRACE("Rendering panel at ({}, {}) size {}x{}", 
        element.bounds.x, element.bounds.y, element.bounds.width, element.bounds.height);
}

void UIManager::RenderDialogueBox() {
    if (!m_dialogue.visible) return;
    
    // Render dialogue background
    Rect boxRect(
        m_screenSize.x * 0.1f,
        m_screenSize.y - 200 * m_uiScale,
        m_screenSize.x * 0.8f,
        180 * m_uiScale
    );
    
    UIElement bg;
    bg.type = UIElementType::PANEL;
    bg.bounds = boxRect;
    bg.backgroundColor = Color(0, 0, 0, 200);
    bg.hasBorder = true;
    bg.borderColor = Color::WHITE();
    bg.cornerRadius = 10.0f;
    RenderPanel(bg);
    
    // Render speaker
    UIElement speaker;
    speaker.type = UIElementType::TEXT;
    speaker.bounds = Rect(boxRect.x + 10, boxRect.y + 10, boxRect.width - 20, 30);
    speaker.text = m_dialogue.speaker;
    speaker.textColor = Color(255, 200, 100);
    speaker.fontSize = 18.0f * m_uiScale;
    RenderText(speaker);
    
    // Render text
    UIElement text;
    text.type = UIElementType::TEXT;
    text.bounds = Rect(boxRect.x + 10, boxRect.y + 45, boxRect.width - 20, boxRect.height - 80);
    text.text = m_dialogue.displayText;
    text.textColor = Color::WHITE();
    text.fontSize = 16.0f * m_uiScale;
    RenderText(text);
    
    // Render choices
    f32 yOffset = boxRect.y + boxRect.height - 50;
    for (i32 i = 0; i < static_cast<i32>(m_dialogue.choices.size()); i++) {
        UIElement choice;
        choice.type = UIElementType::BUTTON;
        choice.bounds = Rect(boxRect.x + 10, yOffset + i * 40, boxRect.width - 20, 35);
        choice.text = m_dialogue.choices[i].first;
        choice.textColor = Color::WHITE();
        choice.fontSize = 14.0f * m_uiScale;
        RenderButton(choice);
    }
}

void UIManager::RenderCrosshair() {
    if (!m_crosshair.visible) return;
    
    // Simple crosshair rendering
    f32 size = m_crosshair.size;
    Color color = m_crosshair.color;
    
    // Draw crosshair lines
    UIElement line1;
    line1.type = UIElementType::PANEL;
    line1.bounds = Rect(
        m_crosshair.position.x - size * 0.5f,
        m_crosshair.position.y - 2,
        size,
        4
    );
    line1.backgroundColor = color;
    RenderPanel(line1);
    
    UIElement line2;
    line2.type = UIElementType::PANEL;
    line2.bounds = Rect(
        m_crosshair.position.x - 2,
        m_crosshair.position.y - size * 0.5f,
        4,
        size
    );
    line2.backgroundColor = color;
    RenderPanel(line2);
}

void UIManager::RenderMinimap() {
    if (!m_minimap.visible) return;
    
    // Render minimap background
    UIElement bg;
    bg.type = UIElementType::PANEL;
    bg.bounds = Rect(
        m_minimap.position.x,
        m_minimap.position.y,
        m_minimap.size,
        m_minimap.size
    );
    bg.backgroundColor = Color(0, 0, 0, 180);
    bg.hasBorder = true;
    bg.borderColor = Color::WHITE();
    bg.cornerRadius = 5.0f;
    RenderPanel(bg);
    
    // TODO: Render minimap content
    // This would render terrain, player position, and markers
}

void UIManager::RenderNotifications() {
    f32 yOffset = 100;
    f32 notificationHeight = 60;
    
    for (const auto& notification : m_notifications) {
        f32 alpha = std::min(1.0f, notification.timer / 0.5f);
        Color color = notification.color;
        color.a = static_cast<u8>(color.a * alpha);
        
        UIElement bg;
        bg.type = UIElementType::PANEL;
        bg.bounds = Rect(
            m_screenSize.x - 300,
            yOffset,
            280,
            notificationHeight - 5
        );
        bg.backgroundColor = Color(0, 0, 0, static_cast<u8>(180 * alpha));
        bg.hasBorder = true;
        bg.borderColor = color;
        bg.cornerRadius = 5.0f;
        RenderPanel(bg);
        
        UIElement text;
        text.type = UIElementType::TEXT;
        text.bounds = Rect(
            m_screenSize.x - 295,
            yOffset + 5,
            270,
            notificationHeight - 15
        );
        text.text = notification.message;
        text.textColor = color;
        text.fontSize = 14.0f;
        RenderText(text);
        
        yOffset += notificationHeight;
    }
}

void UIManager::RenderDebugUI() {
    if (!m_debug.visible) return;
    
    // Render debug panel
    UIElement bg;
    bg.type = UIElementType::PANEL;
    bg.bounds = Rect(10, m_screenSize.y - 300, 300, 280);
    bg.backgroundColor = Color(0, 0, 0, 200);
    bg.hasBorder = true;
    bg.borderColor = Color::GREEN();
    bg.cornerRadius = 5.0f;
    RenderPanel(bg);
    
    // Render debug info
    f32 yOffset = m_screenSize.y - 290;
    for (const auto& [key, value] : m_debug.info) {
        UIElement text;
        text.type = UIElementType::TEXT;
        text.bounds = Rect(20, yOffset, 280, 20);
        text.text = std::format("{}: {}", key, value);
        text.textColor = Color::GREEN();
        text.fontSize = 12.0f;
        RenderText(text);
        yOffset += 20;
    }
}

void UIManager::UpdateNotificationQueue(f32 deltaTime) {
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        it->timer -= deltaTime;
        if (it->timer <= 0.0f) {
            it = m_notifications.erase(it);
        } else {
            ++it;
        }
    }
}

bool UIManager::IsPointInRect(const Vec2& point, const Rect& rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width &&
           point.y >= rect.y && point.y <= rect.y + rect.height;
}

} // namespace Duality