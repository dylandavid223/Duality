// =============================================================================
// FILE: src/Game/Character/ElsySystem.h
// PURPOSE: AI companion voice system with procedural dialogue
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include <functional>
#include <queue>

namespace Duality {

enum class ElsyMood {
    NEUTRAL,
    CONCERNED,
    WORRIED,
    FRUSTRATED,
    MYSTERIOUS,
    HOPEFUL,
    DESPAIRING,
    PLAYFUL
};

enum class ElsyPersonalityMode {
    PRAGMATIC,      // Cold survival calculations
    MORALIZING,     // Judges your choices
    FRAGMENTED,     // Glitches reveal hidden knowledge
    OMINOUS,        // Hints at larger truths
    CARING,         // Brief moments of simulated concern
    TEMPORAL        // References future/past events
};

struct DialogueContext {
    f32 radiationLevel;
    f32 healthPercentage;
    f32 sanityLevel;
    f32 dangerLevel;
    Vec3 position;
    std::string currentBiome;
    bool isInCombat;
    bool isPhaseShifting;
    u32 playTime;
    u32 kills;
    u32 deaths;
    std::vector<std::string> recentEvents;
};

struct DialogueLine {
    std::string text;
    ElsyMood mood;
    f32 priority;
    bool isRepeatable;
    bool isImportant;
    std::function<bool(const DialogueContext&)> condition;
};

class ElsySystem {
public:
    ElsySystem();
    ~ElsySystem();
    
    void Initialize(u64 seed);
    void Shutdown();
    void Update(f32 deltaTime, const DialogueContext& context);
    
    // Dialogue
    void TriggerDialogue(const std::string& topic);
    void RequestComment();
    void AddCustomLine(const DialogueLine& line);
    
    // Knowledge
    void RevealInformation(const std::string& info);
    bool KnowsInformation(const std::string& info) const;
    
    // Personality
    void SetPersonalityMode(ElsyPersonalityMode mode);
    void IncreaseTrust(f32 amount);
    void DecreaseTrust(f32 amount);
    
    // Voice synthesis
    void SetVoiceEnabled(bool enabled) { m_voiceEnabled = enabled; }
    void SetVolume(f32 volume) { m_volume = volume; }
    
    // Callbacks
    using DialogueCallback = std::function<void(const std::string&, ElsyMood)>;
    void SetDialogueCallback(DialogueCallback callback) { m_dialogueCallback = callback; }
    
private:
    void ProcessDialogueQueue();
    void GenerateContextualDialogue(const DialogueContext& context);
    void GenerateRandomDialogue();
    void UpdatePersonality();
    std::string GenerateResponse(const std::string& topic, const DialogueContext& context);
    std::string ApplyVoiceModulation(const std::string& text, ElsyMood mood);
    
    struct KnowledgeNode {
        std::string information;
        f32 trustRequired;
        bool revealed = false;
        std::string dialogueLine;
        std::vector<std::string> relatedTopics;
    };
    
    struct PersonalityState {
        ElsyPersonalityMode mode = ElsyPersonalityMode::PRAGMATIC;
        f32 trustLevel = 0.1f;
        f32 suspicionLevel = 0.0f;
        f32 curiosityLevel = 0.5f;
        f32 empathyLevel = 0.3f;
        f32 stabilityLevel = 0.8f;
    };
    
    std::unordered_map<std::string, KnowledgeNode> m_knowledge;
    std::vector<DialogueLine> m_dialogueLines;
    std::queue<std::pair<std::string, ElsyMood>> m_dialogueQueue;
    
    PersonalityState m_personality;
    ElsyMood m_currentMood = ElsyMood::NEUTRAL;
    f32 m_lastDialogueTime = 0.0f;
    f32 m_minDialogueInterval = 30.0f;
    f32 m_volume = 1.0f;
    bool m_voiceEnabled = true;
    bool m_isSpeaking = false;
    
    Random m_random;
    DialogueCallback m_dialogueCallback;
    
    // Predefined dialogue templates
    std::vector<std::string> m_greetings;
    std::vector<std::string> m_warnings;
    std::vector<std::string> m_observations;
    std::vector<std::string> m_combatLines;
    std::vector<std::string> m_phaseShiftLines;
    std::vector<std::string> m_storyHints;
};

} // namespace Duality