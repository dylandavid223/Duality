// =============================================================================
// FILE: src/Game/ApocalypseGame.cpp
// PURPOSE: Main game implementation
// =============================================================================

#include "ApocalypseGame.h"
#include "Engine/Core/Logger.h"
#include "Engine/Core/Timer.h"
#include "Engine/Math/Math.h"

namespace Duality {

ApocalypseGame::ApocalypseGame() {
    LOG_INFO("ApocalypseGame created");
}

ApocalypseGame::~ApocalypseGame() {
    Shutdown();
}

void ApocalypseGame::Initialize() {
    LOG_INFO("Initializing ApocalypseGame...");
    
    m_engine = Engine::GetInstance();
    
    // Initialize world generator
    m_worldGen = MakeScope<WorldGenerator>();
    m_worldGen->Initialize(21530903ULL, 100000.0f);
    
    // Initialize player
    m_player = MakeScope<PlayerController>();
    m_player->Initialize(m_engine->GetInput());
    
    // Initialize Elsy
    m_elsy = MakeScope<ElsySystem>();
    m_elsy->Initialize(21530903ULL);
    
    // Setup UI
    m_ui = MakeScope<UIManager>();
    m_ui->Initialize(m_engine->GetRenderer(), m_engine->GetWindow());
    
    // Setup input bindings
    SetupInputBindings();
    
    // Set initial player position
    Vec3 startPos = Vec3(0.0f, 50.0f, 0.0f);
    m_player->SetPosition(startPos);
    
    // Generate initial chunks around player
    m_worldGen->UpdateStreaming(startPos, 500.0f);
    
    // Set up projection matrix
    f32 aspect = m_engine->GetWindow()->GetAspectRatio();
    m_projMatrix = Mat4::Perspective(Math::Constants::HALF_PI, aspect, 0.1f, 10000.0f);
    
    // Elsy intro
    m_elsy->TriggerDialogue("greeting");
    
    m_isInitialized = true;
    LOG_INFO("ApocalypseGame initialized successfully");
}

void ApocalypseGame::Shutdown() {
    LOG_INFO("Shutting down ApocalypseGame...");
    
    if (m_player) m_player->Shutdown();
    if (m_elsy) m_elsy->Shutdown();
    if (m_ui) m_ui->Shutdown();
    if (m_worldGen) m_worldGen->Shutdown();
    
    m_isInitialized = false;
    LOG_INFO("ApocalypseGame shutdown complete");
}

void ApocalypseGame::Update(f32 deltaTime) {
    PROFILE_SCOPE("Game Update");
    
    if (!m_isInitialized || m_isPaused || m_isGameOver) return;
    
    m_gameTime += deltaTime;
    
    // Update game systems
    UpdateGameLogic(deltaTime);
    UpdateUI();
    
    // Check for game over
    CheckGameOver();
}

void ApocalypseGame::FixedUpdate(f32 fixedDeltaTime) {
    if (!m_isInitialized || m_isPaused || m_isGameOver) return;
    
    // Fixed timestep updates
    if (m_player) {
        m_player->FixedUpdate(fixedDeltaTime);
    }
}

void ApocalypseGame::Render() {
    PROFILE_SCOPE("Game Render");
    
    if (!m_isInitialized) return;
    
    // Update view matrix from player camera
    m_viewMatrix = Mat4::LookAt(
        m_player->GetCameraPosition(),
        m_player->GetCameraPosition() + m_player->GetCameraForward(),
        Vec3::UP()
    );
    
    // TODO: Submit render commands to renderer
    // This would include terrain, player model, UI, etc.
}

void ApocalypseGame::OnResize(u32 width, u32 height) {
    f32 aspect = static_cast<f32>(width) / static_cast<f32>(height);
    m_projMatrix = Mat4::Perspective(Math::Constants::HALF_PI, aspect, 0.1f, 10000.0f);
    
    if (m_ui) {
        m_ui->OnResize(width, height);
    }
}

void ApocalypseGame::UpdateGameLogic(f32 deltaTime) {
    // Update world streaming based on player position
    if (m_worldGen && m_player) {
        m_worldGen->UpdateStreaming(m_player->GetPosition(), 500.0f);
    }
    
    // Update player
    if (m_player) {
        m_player->Update(deltaTime);
    }
    
    // Update Elsy with current context
    if (m_elsy && m_player) {
        DialogueContext context;
        context.radiationLevel = m_player->GetStats().radiation / 100.0f;
        context.healthPercentage = m_player->GetStats().health / 100.0f;
        context.sanityLevel = m_player->GetStats().sanity / 100.0f;
        context.isInCombat = false; // TODO: Get combat state
        context.isPhaseShifting = m_player->IsPhaseTransitioning();
        context.position = m_player->GetPosition();
        
        BiomeType biome = m_worldGen->GetBiome(context.position.x, context.position.z);
        context.currentBiome = m_worldGen->GetBiomeName(biome);
        
        m_elsy->Update(deltaTime, context);
    }
}

void ApocalypseGame::UpdateUI() {
    if (!m_ui) return;
    
    // Update UI elements with current game state
    if (m_player) {
        m_ui->UpdateHealth(m_player->GetStats().health, m_player->GetStats().maxHealth);
        m_ui->UpdateStamina(m_player->GetStats().stamina, m_player->GetStats().maxStamina);
        m_ui->UpdateRadiation(m_player->GetStats().radiation, m_player->GetStats().maxRadiation);
    }
    
    if (m_elsy) {
        // Elsy dialogue would be shown through UI
    }
    
    m_ui->UpdateObjective(GetCurrentObjective());
    m_ui->UpdateGameTime(m_gameTime);
}

void ApocalypseGame::SetupInputBindings() {
    InputSystem* input = m_engine->GetInput();
    if (!input) return;
    
    // Phase shift binding
    ActionBinding phaseShiftBinding;
    phaseShiftBinding.keys = {KeyCode::Q};
    phaseShiftBinding.onPress = [this]() { OnPhaseShift(); };
    input->BindAction("PhaseShift", phaseShiftBinding);
    
    // Interact binding
    ActionBinding interactBinding;
    interactBinding.keys = {KeyCode::E};
    interactBinding.onPress = [this]() { OnInteract(); };
    input->BindAction("Interact", interactBinding);
    
    // Pause binding
    ActionBinding pauseBinding;
    pauseBinding.keys = {KeyCode::Escape};
    pauseBinding.onPress = [this]() { m_isPaused = !m_isPaused; };
    input->BindAction("Pause", pauseBinding);
}

void ApocalypseGame::OnPhaseShift() {
    if (m_player && !m_player->IsPhaseTransitioning()) {
        m_player->TriggerPhaseShift();
        
        if (m_elsy) {
            m_elsy->TriggerDialogue("phase_shift");
        }
        
        LOG_INFO("Phase shift triggered by player");
    }
}

void ApocalypseGame::OnInteract() {
    if (m_player) {
        m_player->Interact();
    }
}

void ApocalypseGame::CheckGameOver() {
    if (m_player && m_player->IsDead()) {
        m_isGameOver = true;
        
        if (m_elsy) {
            m_elsy->TriggerDialogue("game_over");
        }
        
        LOG_INFO("Game Over! Final score: {}", m_score);
    }
}

void ApocalypseGame::SaveGame(const std::string& filename) {
    LOG_INFO("Saving game to: {}", filename);
    
    // TODO: Implement save game serialization
    // This would save player position, stats, world state, etc.
}

void ApocalypseGame::LoadSaveGame(const std::string& filename) {
    LOG_INFO("Loading game from: {}", filename);
    
    // TODO: Implement save game deserialization
}

void ApocalypseGame::AutoSave() {
    std::string filename = "autosave_" + std::to_string(static_cast<i64>(m_gameTime)) + ".sav";
    SaveGame(filename);
}

std::string ApocalypseGame::GetCurrentObjective() const {
    // TODO: Implement dynamic objective system
    return "Survive. Find a way off Earth.";
}

bool ApocalypseGame::IsGameOver() const {
    return m_isGameOver;
}

bool ApocalypseGame::IsPaused() const {
    return m_isPaused;
}

} // namespace Duality