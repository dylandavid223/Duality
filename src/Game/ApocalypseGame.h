// =============================================================================
// FILE: src/Game/ApocalypseGame.h
// PURPOSE: Main game class orchestrating all systems
// =============================================================================

#pragma once

#include "Engine/Core/Engine.h"
#include "Game/World/WorldGenerator.h"
#include "Game/Character/PlayerController.h"
#include "Game/Character/ElsySystem.h"
#include "Game/UI/UIManager.h"

namespace Duality {

class ApocalypseGame : public IGameInstance {
public:
    ApocalypseGame();
    ~ApocalypseGame() override;
    
    // IGameInstance implementation
    void Initialize() override;
    void Shutdown() override;
    void Update(f32 deltaTime) override;
    void FixedUpdate(f32 fixedDeltaTime) override;
    void Render() override;
    void OnResize(u32 width, u32 height) override;
    
    void SaveGame(const std::string& filename) override;
    void LoadSaveGame(const std::string& filename) override;
    void AutoSave() override;
    
    std::string GetCurrentObjective() const override;
    bool IsGameOver() const override;
    bool IsPaused() const override;
    
private:
    void UpdateGameLogic(f32 deltaTime);
    void UpdateUI();
    void ProcessEvents();
    void CheckGameOver();
    
    void SetupInputBindings();
    void OnPhaseShift();
    void OnInteract();
    
    // Core systems
    Engine* m_engine = nullptr;
    Scope<WorldGenerator> m_worldGen;
    Scope<PlayerController> m_player;
    Scope<ElsySystem> m_elsy;
    Scope<UIManager> m_ui;
    
    // Game state
    bool m_isInitialized = false;
    bool m_isGameOver = false;
    bool m_isPaused = false;
    f32 m_gameTime = 0.0f;
    u32 m_score = 0;
    u32 m_kills = 0;
    
    // Camera
    Mat4 m_viewMatrix;
    Mat4 m_projMatrix;
    
    // Performance
    f32 m_updateTime = 0.0f;
    f32 m_renderTime = 0.0f;
    u32 m_frameCount = 0;
};

} // namespace Duality