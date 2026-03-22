// =============================================================================
// FILE: src/Game/Character/PlayerController.h
// PURPOSE: Player movement, phase shifting, and interaction
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include "Engine/Input/InputSystem.h"

namespace Duality {

enum class PhaseState {
    APOCALYPSE = 0,
    ART_CITY = 1,
    TRANSITION = 2
};

struct PlayerStats {
    f32 health = 100.0f;
    f32 maxHealth = 100.0f;
    f32 stamina = 100.0f;
    f32 maxStamina = 100.0f;
    f32 radiation = 0.0f;
    f32 maxRadiation = 100.0f;
    f32 hunger = 0.0f;
    f32 thirst = 0.0f;
    f32 sanity = 100.0f;
    f32 maxSanity = 100.0f;
    
    f32 movementSpeed = 5.0f;
    f32 sprintSpeed = 10.0f;
    f32 crouchSpeed = 2.5f;
    f32 jumpForce = 8.0f;
    
    bool isCrouching = false;
    bool isSprinting = false;
    bool isAiming = false;
    bool isInjured = false;
};

struct PhaseShiftData {
    PhaseState currentPhase = PhaseState::APOCALYPSE;
    PhaseState targetPhase = PhaseState::APOCALYPSE;
    f32 transitionProgress = 0.0f;
    f32 transitionDuration = 1.5f;
    bool isTransitioning = false;
    f32 cooldown = 0.0f;
    f32 maxCooldown = 3.0f;
};

class PlayerController {
public:
    PlayerController();
    ~PlayerController();
    
    void Initialize(InputSystem* input);
    void Shutdown();
    
    void Update(f32 deltaTime);
    void FixedUpdate(f32 fixedDeltaTime);
    
    // Movement
    void Move(const Vec3& direction);
    void Jump();
    void Crouch(bool crouch);
    void Sprint(bool sprint);
    void Aim(bool aim);
    
    // Phase shifting
    void TriggerPhaseShift();
    void UpdatePhaseTransition(f32 deltaTime);
    [[nodiscard]] PhaseState GetCurrentPhase() const { return m_phaseData.currentPhase; }
    [[nodiscard]] f32 GetPhaseBlend() const { return m_phaseData.transitionProgress; }
    [[nodiscard]] bool IsPhaseTransitioning() const { return m_phaseData.isTransitioning; }
    
    // Camera
    void SetCameraPosition(const Vec3& position);
    void SetCameraRotation(const Vec3& rotation);
    void UpdateCamera(f32 deltaTime);
    [[nodiscard]] Vec3 GetCameraPosition() const { return m_cameraPosition; }
    [[nodiscard]] Vec3 GetCameraForward() const;
    [[nodiscard]] Vec3 GetCameraRight() const;
    
    // Stats
    void ApplyDamage(f32 damage);
    void ApplyRadiation(f32 radiation);
    void Heal(f32 amount);
    void ConsumeStamina(f32 amount);
    void RegenerateStamina(f32 deltaTime);
    
    [[nodiscard]] const PlayerStats& GetStats() const { return m_stats; }
    [[nodiscard]] bool IsAlive() const { return m_stats.health > 0.0f; }
    [[nodiscard]] bool IsDead() const { return m_stats.health <= 0.0f; }
    
    // Interaction
    void Interact();
    void SetInteractionDistance(f32 distance) { m_interactionDistance = distance; }
    
private:
    void UpdateMovement(f32 deltaTime);
    void UpdatePhysics(f32 deltaTime);
    void HandleInput();
    void ApplyGravity(f32 deltaTime);
    void CheckGroundCollision();
    
    InputSystem* m_input = nullptr;
    
    // Transform
    Vec3 m_position = Vec3::ZERO();
    Vec3 m_velocity = Vec3::ZERO();
    Vec3 m_cameraPosition = Vec3::ZERO();
    Vec3 m_cameraRotation = Vec3::ZERO();
    
    Quat m_rotation = Quat::IDENTITY();
    
    // State
    PlayerStats m_stats;
    PhaseShiftData m_phaseData;
    bool m_isGrounded = true;
    bool m_isMoving = false;
    f32 m_footstepTimer = 0.0f;
    
    // Physics
    f32 m_gravity = -20.0f;
    f32 m_groundCheckDistance = 0.2f;
    f32 m_interactionDistance = 3.0f;
    
    // Input actions
    struct InputBindings {
        std::string moveForward = "MoveForward";
        std::string moveBack = "MoveBack";
        std::string moveLeft = "MoveLeft";
        std::string moveRight = "MoveRight";
        std::string jump = "Jump";
        std::string crouch = "Crouch";
        std::string sprint = "Sprint";
        std::string aim = "Aim";
        std::string interact = "Interact";
        std::string phaseShift = "PhaseShift";
    } m_bindings;
};

} // namespace Duality