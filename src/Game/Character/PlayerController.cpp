// =============================================================================
// FILE: src/Game/Character/PlayerController.cpp
// PURPOSE: Player controller implementation
// =============================================================================

#include "PlayerController.h"
#include "Engine/Core/Logger.h"
#include "Engine/Math/Math.h"

namespace Duality {

PlayerController::PlayerController() {
    LOG_INFO("Player controller created");
}

PlayerController::~PlayerController() {
    Shutdown();
}

void PlayerController::Initialize(InputSystem* input) {
    m_input = input;
    
    // Setup input bindings
    if (m_input) {
        // Movement axes
        AxisBinding moveHorizontal;
        moveHorizontal.keys = {{KeyCode::D, 1.0f}, {KeyCode::A, -1.0f}};
        moveHorizontal.sensitivity = 1.0f;
        moveHorizontal.deadzone = 0.1f;
        m_input->BindAxis(m_bindings.moveRight, moveHorizontal);
        
        AxisBinding moveVertical;
        moveVertical.keys = {{KeyCode::W, 1.0f}, {KeyCode::S, -1.0f}};
        moveVertical.sensitivity = 1.0f;
        moveVertical.deadzone = 0.1f;
        m_input->BindAxis(m_bindings.moveForward, moveVertical);
        
        // Action bindings
        ActionBinding jumpBinding;
        jumpBinding.keys = {KeyCode::Space};
        jumpBinding.onPress = [this]() { Jump(); };
        m_input->BindAction(m_bindings.jump, jumpBinding);
        
        ActionBinding crouchBinding;
        crouchBinding.keys = {KeyCode::LeftControl};
        crouchBinding.onPress = [this]() { Crouch(true); };
        crouchBinding.onRelease = [this]() { Crouch(false); };
        m_input->BindAction(m_bindings.crouch, crouchBinding);
        
        ActionBinding sprintBinding;
        sprintBinding.keys = {KeyCode::LeftShift};
        sprintBinding.onPress = [this]() { Sprint(true); };
        sprintBinding.onRelease = [this]() { Sprint(false); };
        m_input->BindAction(m_bindings.sprint, sprintBinding);
        
        ActionBinding aimBinding;
        aimBinding.keys = {KeyCode::MouseRight};
        aimBinding.onPress = [this]() { Aim(true); };
        aimBinding.onRelease = [this]() { Aim(false); };
        m_input->BindAction(m_bindings.aim, aimBinding);
        
        ActionBinding interactBinding;
        interactBinding.keys = {KeyCode::E};
        interactBinding.onPress = [this]() { Interact(); };
        m_input->BindAction(m_bindings.interact, interactBinding);
        
        ActionBinding phaseShiftBinding;
        phaseShiftBinding.keys = {KeyCode::Q};
        phaseShiftBinding.onPress = [this]() { TriggerPhaseShift(); };
        m_input->BindAction(m_bindings.phaseShift, phaseShiftBinding);
    }
    
    LOG_INFO("Player controller initialized");
}

void PlayerController::Shutdown() {
    LOG_INFO("Player controller shutdown");
}

void PlayerController::Update(f32 deltaTime) {
    if (IsDead()) return;
    
    HandleInput();
    UpdateMovement(deltaTime);
    UpdatePhysics(deltaTime);
    UpdateCamera(deltaTime);
    UpdatePhaseTransition(deltaTime);
    
    // Update footstep sounds
    if (m_isMoving && m_isGrounded && !m_stats.isCrouching) {
        m_footstepTimer += deltaTime;
        f32 stepInterval = m_stats.isSprinting ? 0.3f : 0.5f;
        if (m_footstepTimer >= stepInterval) {
            m_footstepTimer = 0.0f;
            // TODO: Play footstep sound
        }
    }
    
    // Regenerate stamina
    if (!m_stats.isSprinting && m_isGrounded) {
        RegenerateStamina(deltaTime);
    }
}

void PlayerController::FixedUpdate(f32 fixedDeltaTime) {
    // Physics updates at fixed timestep
    ApplyGravity(fixedDeltaTime);
    CheckGroundCollision();
}

void PlayerController::HandleInput() {
    if (!m_input) return;
    
    f32 moveX = m_input->GetAxis(m_bindings.moveRight);
    f32 moveZ = m_input->GetAxis(m_bindings.moveForward);
    
    if (moveX != 0.0f || moveZ != 0.0f) {
        m_isMoving = true;
        
        Vec3 direction = GetCameraRight() * moveX + GetCameraForward() * moveZ;
        direction.y = 0.0f;
        direction = direction.Normalized();
        
        Move(direction);
    } else {
        m_isMoving = false;
    }
}

void PlayerController::UpdateMovement(f32 deltaTime) {
    f32 speed = m_stats.movementSpeed;
    if (m_stats.isSprinting && m_stats.stamina > 0.0f) {
        speed = m_stats.sprintSpeed;
        ConsumeStamina(deltaTime * 20.0f);
    } else if (m_stats.isCrouching) {
        speed = m_stats.crouchSpeed;
    }
    
    // Apply movement velocity
    m_velocity.x = m_moveDirection.x * speed;
    m_velocity.z = m_moveDirection.z * speed;
    
    // Update position
    m_position += m_velocity * deltaTime;
}

void PlayerController::UpdatePhysics(f32 deltaTime) {
    // Apply movement to camera position
    m_cameraPosition = m_position + Vec3(0.0f, 1.7f, 0.0f);
    
    if (m_stats.isCrouching) {
        m_cameraPosition.y = m_position.y + 1.0f;
    }
}

void PlayerController::UpdateCamera(f32 deltaTime) {
    if (!m_input) return;
    
    // Mouse look
    Vec2 mouseDelta = m_input->GetMouseDelta();
    
    f32 sensitivity = m_stats.isAiming ? 0.5f : 1.0f;
    m_cameraRotation.x += mouseDelta.y * 0.002f * sensitivity;
    m_cameraRotation.y += mouseDelta.x * 0.002f * sensitivity;
    
    // Clamp pitch
    m_cameraRotation.x = std::clamp(m_cameraRotation.x, -Math::Constants::HALF_PI, Math::Constants::HALF_PI);
    
    // Update camera rotation quaternion
    m_rotation = Quat::FromEuler(m_cameraRotation.x, m_cameraRotation.y, 0.0f);
}

void PlayerController::Move(const Vec3& direction) {
    m_moveDirection = direction;
}

void PlayerController::Jump() {
    if (m_isGrounded && !m_stats.isCrouching) {
        m_velocity.y = m_stats.jumpForce;
        m_isGrounded = false;
        // TODO: Play jump sound
    }
}

void PlayerController::Crouch(bool crouch) {
    if (crouch && !m_stats.isCrouching && m_isGrounded) {
        m_stats.isCrouching = true;
        m_stats.isSprinting = false;
        // TODO: Play crouch sound
    } else if (!crouch && m_stats.isCrouching) {
        m_stats.isCrouching = false;
    }
}

void PlayerController::Sprint(bool sprint) {
    if (sprint && !m_stats.isCrouching && m_stats.stamina > 0.0f) {
        m_stats.isSprinting = true;
    } else if (!sprint) {
        m_stats.isSprinting = false;
    }
}

void PlayerController::Aim(bool aim) {
    m_stats.isAiming = aim;
}

void PlayerController::TriggerPhaseShift() {
    if (m_phaseData.isTransitioning) return;
    if (m_phaseData.cooldown > 0.0f) return;
    
    m_phaseData.isTransitioning = true;
    m_phaseData.transitionProgress = 0.0f;
    m_phaseData.targetPhase = (m_phaseData.currentPhase == PhaseState::APOCALYPSE) 
        ? PhaseState::ART_CITY 
        : PhaseState::APOCALYPSE;
    
    LOG_INFO("Phase shift triggered: {} -> {}", 
        static_cast<int>(m_phaseData.currentPhase),
        static_cast<int>(m_phaseData.targetPhase));
}

void PlayerController::UpdatePhaseTransition(f32 deltaTime) {
    if (!m_phaseData.isTransitioning) {
        if (m_phaseData.cooldown > 0.0f) {
            m_phaseData.cooldown -= deltaTime;
        }
        return;
    }
    
    m_phaseData.transitionProgress += deltaTime / m_phaseData.transitionDuration;
    
    if (m_phaseData.transitionProgress >= 1.0f) {
        m_phaseData.transitionProgress = 1.0f;
        m_phaseData.isTransitioning = false;
        m_phaseData.currentPhase = m_phaseData.targetPhase;
        m_phaseData.cooldown = m_phaseData.maxCooldown;
        
        LOG_INFO("Phase shift complete: {}", static_cast<int>(m_phaseData.currentPhase));
    }
}

Vec3 PlayerController::GetCameraForward() const {
    return m_rotation * Vec3::FORWARD();
}

Vec3 PlayerController::GetCameraRight() const {
    return m_rotation * Vec3::RIGHT();
}

void PlayerController::ApplyGravity(f32 deltaTime) {
    if (!m_isGrounded) {
        m_velocity.y += m_gravity * deltaTime;
    } else {
        m_velocity.y = 0.0f;
    }
}

void PlayerController::CheckGroundCollision() {
    // Simple ground check - raycast downward
    Vec3 rayStart = m_position + Vec3(0.0f, 0.1f, 0.0f);
    Vec3 rayEnd = rayStart - Vec3(0.0f, m_groundCheckDistance, 0.0f);
    
    // TODO: Implement proper raycast against terrain
    // For now, assume ground is at y=0
    m_isGrounded = m_position.y <= 0.1f;
    
    if (m_isGrounded && m_position.y < 0.0f) {
        m_position.y = 0.0f;
        m_velocity.y = 0.0f;
    }
}

void PlayerController::ApplyDamage(f32 damage) {
    m_stats.health = std::max(0.0f, m_stats.health - damage);
    
    if (m_stats.health <= 0.0f) {
        LOG_INFO("Player died");
        // TODO: Handle death
    }
}

void PlayerController::ApplyRadiation(f32 radiation) {
    m_stats.radiation = std::min(m_stats.maxRadiation, m_stats.radiation + radiation);
    
    if (m_stats.radiation > m_stats.maxRadiation * 0.7f) {
        ApplyDamage(radiation * 0.1f);
    }
}

void PlayerController::Heal(f32 amount) {
    m_stats.health = std::min(m_stats.maxHealth, m_stats.health + amount);
}

void PlayerController::ConsumeStamina(f32 amount) {
    m_stats.stamina = std::max(0.0f, m_stats.stamina - amount);
    
    if (m_stats.stamina <= 0.0f) {
        m_stats.isSprinting = false;
    }
}

void PlayerController::RegenerateStamina(f32 deltaTime) {
    if (m_stats.stamina < m_stats.maxStamina && !m_stats.isSprinting) {
        m_stats.stamina = std::min(m_stats.maxStamina, m_stats.stamina + deltaTime * 15.0f);
    }
}

void PlayerController::Interact() {
    LOG_DEBUG("Player interacting");
    // TODO: Implement interaction system
}

} // namespace Duality