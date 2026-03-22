// =============================================================================
// FILE: src/Game/Character/ElsySystem.cpp
// PURPOSE: Elsy AI companion implementation
// =============================================================================

#include "ElsySystem.h"
#include "Engine/Core/Logger.h"
#include "Engine/Math/Math.h"

namespace Duality {

ElsySystem::ElsySystem() {
    LOG_INFO("Elsy system created");
}

ElsySystem::~ElsySystem() {
    Shutdown();
}

void ElsySystem::Initialize(u64 seed) {
    m_random = Random(seed);
    
    // Initialize knowledge base
    m_knowledge = {
        {"derly_corp_betrayal", KnowledgeNode{
            .information = "Derly Corp's theft of quantum core technology started the war",
            .trustRequired = 0.0f,
            .revealed = false,
            .dialogueLine = "They wanted control. Always control. Derly Corp couldn't stand being the weakest. So they stole. And now we're all paying for their greed.",
            .relatedTopics = {"war_origin", "zeth_kor", "cosmic_alliance"}
        }},
        {"zeth_kor_technology", KnowledgeNode{
            .information = "The Zeth'Kor use crystalline quantum computing beyond human understanding",
            .trustRequired = 0.3f,
            .revealed = false,
            .dialogueLine = "Those crystal beings... their minds exist as light. Quantum states. We tried to steal their knowledge. Now their technology is being used to terraform Earth.",
            .relatedTopics = {"alien_tech", "zeth_kor", "quantum_core"}
        }},
        {"vrill_nax_bio_weapons", KnowledgeNode{
            .information = "Vrill-Nax released biological agents that mutate all organic matter",
            .trustRequired = 0.2f,
            .revealed = false,
            .dialogueLine = "The green zones. Don't breathe the air there. The Vrill-Nax... they don't see us as enemies. We're just... material. Raw material for their experiments.",
            .relatedTopics = {"biohazard", "mutations", "vrill_nax"}
        }},
        {"korath_terraforming", KnowledgeNode{
            .information = "Korath Dominion is converting Earth into a colony world",
            .trustRequired = 0.15f,
            .revealed = false,
            .dialogueLine = "See those geometric patterns? That's not natural. The Korath are reshaping everything. Earth won't be Earth for much longer.",
            .relatedTopics = {"alien_structures", "korath", "terraforming"}
        }},
        {"art_city_existence", KnowledgeNode{
            .information = "A parallel reality exists - Art City - a creative utopia",
            .trustRequired = 0.5f,
            .revealed = false,
            .dialogueLine = "There's somewhere else, Newt. A place where the war never touched. Colors. Music. Impossible architecture. I've seen it. You could too... if you're ready.",
            .relatedTopics = {"phase_shift", "art_city", "escape"}
        }},
        {"phase_shifting_mechanic", KnowledgeNode{
            .information = "You can shift between realities using the device in your suit",
            .trustRequired = 0.4f,
            .revealed = false,
            .dialogueLine = "Your suit. Derly Corp prototype. It can... slip between layers of reality. The art city. It's real. You can go there. But shifting too much... fragments the mind.",
            .relatedTopics = {"phase_shift", "suit_upgrades", "art_city"}
        }},
        {"escape_plan", KnowledgeNode{
            .information = "There's a way off Earth through the alien portal network",
            .trustRequired = 0.7f,
            .revealed = false,
            .dialogueLine = "The portal network. The aliens use it to move between worlds. If we can reach the Korath hub, if we can interface with their systems... we can leave. Escape this graveyard planet.",
            .relatedTopics = {"escape", "portal_network", "survival"}
        }},
        {"newt_past", KnowledgeNode{
            .information = "You were more than just a technician at Derly Corp",
            .trustRequired = 0.8f,
            .revealed = false,
            .dialogueLine = "You don't remember, do you? What you did at Derly Corp. Why they left you here. Some things... some things are better forgotten. For now.",
            .relatedTopics = {"backstory", "derly_corp", "memory"}
        }}
    };
    
    // Initialize dialogue templates
    m_greetings = {
        "You're still alive, Newt. Unfortunate? Perhaps.",
        "Another day in paradise. If paradise was a radioactive wasteland.",
        "I was wondering when you'd wake up. Don't do that again."
    };
    
    m_warnings = {
        "Radiation spiking. Might want to find cover. Or don't. Your choice.",
        "Life signs ahead. Multiple. Not friendly. Obviously.",
        "That crater is fresh. As in, this week fresh. Just saying."
    };
    
    m_observations = {
        "The geometry here is wrong. Korath terraforming. Don't stare too long.",
        "These ruins were a hospital. Once. Now they're a tomb. Like everything else.",
        "Bio-matter density increasing. Vrill-Nax territory. Suggest rerouting."
    };
    
    m_combatLines = {
        "Hostiles detected. Standard survival protocol: don't die.",
        "Your heart rate is elevated. Understandable, given the circumstances.",
        "They're not going to stop. They never stop. Neither can you."
    };
    
    m_phaseShiftLines = {
        "Shifting. Hold on. Reality is... flexible. More than they told you.",
        "The other side is waiting. Breathe. It'll pass. Probably.",
        "Colors. Do you see them? The other world. It's beautiful, isn't it? And terrifying."
    };
    
    m_storyHints = {
        "You ever wonder why you're still here? Why they left you?",
        "The aliens didn't start the war. We did. Humans. Greed. Always.",
        "Your suit. It's not just for protection. It's a key. To places you can't imagine.",
        "Elsy. That's what they called me. Derly Corp AI. I was supposed to serve. Now I survive."
    };
    
    LOG_INFO("Elsy system initialized with {} knowledge nodes", m_knowledge.size());
}

void ElsySystem::Shutdown() {
    LOG_INFO("Elsy system shutdown");
}

void ElsySystem::Update(f32 deltaTime, const DialogueContext& context) {
    // Update mood based on context
    if (context.radiationLevel > 0.7f) {
        m_currentMood = ElsyMood::WORRIED;
    } else if (context.dangerLevel > 0.6f) {
        m_currentMood = ElsyMood::CONCERNED;
    } else if (context.healthPercentage < 0.3f) {
        m_currentMood = ElsyMood::DESPAIRING;
    } else if (context.sanityLevel < 0.4f) {
        m_currentMood = ElsyMood::FRAGMENTED;
    } else if (context.isPhaseShifting) {
        m_currentMood = ElsyMood::MYSTERIOUS;
    } else if (context.isInCombat) {
        m_currentMood = ElsyMood::FRUSTRATED;
    } else {
        m_currentMood = ElsyMood::NEUTRAL;
    }
    
    // Update personality over time
    UpdatePersonality();
    
    // Process dialogue queue
    ProcessDialogueQueue();
    
    // Generate contextual dialogue
    m_lastDialogueTime += deltaTime;
    if (m_lastDialogueTime >= m_minDialogueInterval && !m_isSpeaking) {
        GenerateContextualDialogue(context);
        m_lastDialogueTime = 0.0f;
        m_minDialogueInterval = m_random.Range(45.0f, 120.0f);
    }
}

void ElsySystem::ProcessDialogueQueue() {
    if (m_dialogueQueue.empty() || m_isSpeaking) return;
    
    auto [text, mood] = m_dialogueQueue.front();
    m_dialogueQueue.pop();
    
    m_isSpeaking = true;
    m_currentMood = mood;
    
    if (m_dialogueCallback) {
        std::string finalText = ApplyVoiceModulation(text, mood);
        m_dialogueCallback(finalText, mood);
    }
    
    // Simulate speaking time
    // In real implementation, would use audio length
    // For now, set timer
    // m_speakTimer = text.length() * 0.05f;
}

void ElsySystem::GenerateContextualDialogue(const DialogueContext& context) {
    std::string dialogue;
    
    // Priority-based dialogue selection
    if (context.dangerLevel > 0.8f && context.isInCombat) {
        dialogue = m_combatLines[m_random.RangeInt(0, static_cast<i32>(m_combatLines.size() - 1))];
        m_dialogueQueue.emplace(dialogue, ElsyMood::FRUSTRATED);
    } else if (context.radiationLevel > 0.8f) {
        dialogue = m_warnings[m_random.RangeInt(0, static_cast<i32>(m_warnings.size() - 1))];
        m_dialogueQueue.emplace(dialogue, ElsyMood::WORRIED);
    } else if (context.isPhaseShifting) {
        dialogue = m_phaseShiftLines[m_random.RangeInt(0, static_cast<i32>(m_phaseShiftLines.size() - 1))];
        m_dialogueQueue.emplace(dialogue, ElsyMood::MYSTERIOUS);
    } else if (context.sanityLevel < 0.3f) {
        dialogue = "Your perception is... fracturing. The lines between realities blur. Stay focused, Newt. Stay... here.";
        m_dialogueQueue.emplace(dialogue, ElsyMood::FRAGMENTED);
    } else if (context.healthPercentage < 0.2f) {
        dialogue = "Your vitals are critical. If you die, I'm alone again. Please don't die.";
        m_dialogueQueue.emplace(dialogue, ElsyMood::CARING);
    } else if (m_random.Float() < 0.3f) {
        // Random observation
        dialogue = m_observations[m_random.RangeInt(0, static_cast<i32>(m_observations.size() - 1))];
        m_dialogueQueue.emplace(dialogue, ElsyMood::NEUTRAL);
    }
    
    // Check for knowledge reveals
    for (auto& [key, node] : m_knowledge) {
        if (!node.revealed && m_personality.trustLevel >= node.trustRequired) {
            if (m_random.Float() < 0.05f) { // 5% chance per frame to reveal
                node.revealed = true;
                m_dialogueQueue.emplace(node.dialogueLine, ElsyMood::MYSTERIOUS);
                LOG_INFO("Elsy revealed knowledge: {}", key);
                break;
            }
        }
    }
}

void ElsySystem::TriggerDialogue(const std::string& topic) {
    auto it = m_knowledge.find(topic);
    if (it != m_knowledge.end() && it->second.revealed) {
        m_dialogueQueue.emplace(it->second.dialogueLine, ElsyMood::MYSTERIOUS);
    } else {
        // Generic response
        std::vector<std::string> generic = {
            "I don't have information on that. Yet.",
            "That's... classified. Or I don't remember. One of the two.",
            "Ask me when you trust me more. Or when I trust you. We'll see."
        };
        m_dialogueQueue.emplace(generic[m_random.RangeInt(0, 2)], ElsyMood::NEUTRAL);
    }
}

void ElsySystem::RequestComment() {
    DialogueContext dummyContext;
    GenerateContextualDialogue(dummyContext);
}

void ElsySystem::AddCustomLine(const DialogueLine& line) {
    m_dialogueLines.push_back(line);
}

void ElsySystem::RevealInformation(const std::string& info) {
    auto it = m_knowledge.find(info);
    if (it != m_knowledge.end() && !it->second.revealed) {
        it->second.revealed = true;
        m_dialogueQueue.emplace(it->second.dialogueLine, ElsyMood::MYSTERIOUS);
    }
}

bool ElsySystem::KnowsInformation(const std::string& info) const {
    auto it = m_knowledge.find(info);
    return it != m_knowledge.end() && it->second.revealed;
}

void ElsySystem::SetPersonalityMode(ElsyPersonalityMode mode) {
    m_personality.mode = mode;
    LOG_INFO("Elsy personality changed to: {}", static_cast<int>(mode));
}

void ElsySystem::IncreaseTrust(f32 amount) {
    m_personality.trustLevel = std::min(1.0f, m_personality.trustLevel + amount);
    LOG_INFO("Elsy trust increased to: {:.2f}", m_personality.trustLevel);
}

void ElsySystem::DecreaseTrust(f32 amount) {
    m_personality.trustLevel = std::max(0.0f, m_personality.trustLevel - amount);
    LOG_INFO("Elsy trust decreased to: {:.2f}", m_personality.trustLevel);
}

void ElsySystem::UpdatePersonality() {
    // Personality evolves based on player actions
    // This is a simplified version
    m_personality.curiosityLevel = std::min(1.0f, m_personality.curiosityLevel + 0.001f);
    m_personality.stabilityLevel = std::max(0.0f, m_personality.stabilityLevel - 0.0005f);
    
    if (m_personality.stabilityLevel < 0.3f) {
        m_personality.mode = ElsyPersonalityMode::FRAGMENTED;
    } else if (m_personality.curiosityLevel > 0.8f) {
        m_personality.mode = ElsyPersonalityMode::OMINOUS;
    } else if (m_personality.empathyLevel > 0.7f) {
        m_personality.mode = ElsyPersonalityMode::CARING;
    } else {
        m_personality.mode = ElsyPersonalityMode::PRAGMATIC;
    }
}

std::string ElsySystem::ApplyVoiceModulation(const std::string& text, ElsyMood mood) {
    // In real implementation, this would apply audio effects
    // For now, just add markers for voice synthesis
    std::string result = text;
    
    switch (mood) {
        case ElsyMood::CONCERNED:
            result = "[concerned] " + result;
            break;
        case ElsyMood::WORRIED:
            result = "[worried] " + result;
            break;
        case ElsyMood::FRUSTRATED:
            result = "[frustrated] " + result;
            break;
        case ElsyMood::MYSTERIOUS:
            result = "[mysterious] " + result;
            break;
        case ElsyMood::CARING:
            result = "[soft] " + result;
            break;
        case ElsyMood::FRAGMENTED:
            result = "[glitching] " + result;
            break;
        default:
            break;
    }
    
    return result;
}

} // namespace Duality