// =============================================================================
// FILE: src/Engine/Core/Timer.cpp
// PURPOSE: Profiler implementation
// =============================================================================

#include "Timer.h"
#include "Logger.h"

namespace Duality {

static Profiler* g_profiler = nullptr;

void Profiler::Initialize() {
    if (!g_profiler) {
        g_profiler = new Profiler();
        LOG_INFO("Profiler initialized");
    }
}

void Profiler::Shutdown() {
    if (g_profiler) {
        g_profiler->DumpStats();
        delete g_profiler;
        g_profiler = nullptr;
    }
}

Profiler& Profiler::Get() {
    return *g_profiler;
}

void Profiler::BeginFrame() {
    m_frameStart = Clock::now();
}

void Profiler::EndFrame() {
    auto now = Clock::now();
    m_frameTime = std::chrono::duration<f32>(now - m_frameStart).count();
    
    // Calculate moving average FPS
    m_fpsAccumulator += m_frameTime;
    m_frameCount++;
    
    if (m_fpsAccumulator >= 1.0f) {
        m_fps = m_frameCount / m_fpsAccumulator;
        m_fpsAccumulator = 0.0f;
        m_frameCount = 0;
    }
}

void Profiler::PushScope(const char* name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scopeStack.push(name);
}

void Profiler::PopScope(const char* name, f64 microseconds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_scopeStack.empty()) {
        m_scopeStack.pop();
    }
    
    auto& entry = m_data[name];
    entry.name = name;
    entry.callCount++;
    entry.totalTime += microseconds;
    entry.maxTime = std::max(entry.maxTime, microseconds);
    entry.minTime = std::min(entry.minTime, microseconds);
    entry.depth = m_scopeStack.size();
}

void Profiler::ResetStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_data.clear();
}

void Profiler::DumpStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    LOG_INFO("=== Profiler Statistics ===");
    LOG_INFO("Frame Time: {:.3f} ms (FPS: {:.1f})", m_frameTime * 1000.0f, m_fps);
    LOG_INFO("");
    LOG_INFO("--- Profile Entries ---");
    
    // Sort by total time descending
    std::vector<const ProfileEntry*> entries;
    for (const auto& [_, entry] : m_data) {
        entries.push_back(&entry);
    }
    std::sort(entries.begin(), entries.end(),
        [](const ProfileEntry* a, const ProfileEntry* b) {
            return a->totalTime > b->totalTime;
        });
    
    for (const auto* entry : entries) {
        LOG_INFO("  {}: {} calls, {:.2f} ms total, {:.2f} ms avg, {:.2f} ms max",
            entry->name, entry->callCount,
            entry->totalTime / 1000.0,
            entry->AverageTime() / 1000.0,
            entry->maxTime / 1000.0);
    }
}

void Profiler::DrawUI(bool* open) {
    // ImGui implementation would go here
    // This is a placeholder
}

} // namespace Duality