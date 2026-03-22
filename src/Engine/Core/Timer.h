// =============================================================================
// FILE: src/Engine/Core/Timer.h
// PURPOSE: High-precision timing and profiling
// =============================================================================

#pragma once

#include "Types.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <stack>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

namespace Duality {

// ============================================================================
// Timer
// ============================================================================

class Timer {
public:
    Timer() { Reset(); }
    
    void Reset() {
        m_start = Clock::now();
        m_lastFrame = m_start;
    }
    
    [[nodiscard]] f32 GetDeltaTime() {
        auto now = Clock::now();
        f32 dt = std::chrono::duration<f32>(now - m_lastFrame).count();
        m_lastFrame = now;
        return dt;
    }
    
    [[nodiscard]] f32 GetElapsed() const {
        return std::chrono::duration<f32>(Clock::now() - m_start).count();
    }
    
    [[nodiscard]] f64 GetElapsedMicro() const {
        return std::chrono::duration<f64, std::micro>(Clock::now() - m_start).count();
    }
    
    static void Sleep(f32 seconds) {
        std::this_thread::sleep_for(std::chrono::duration<f32>(seconds));
    }
    
    static void PreciseSleep(f32 seconds) {
        auto start = Clock::now();
        auto end = start + std::chrono::duration<f32>(seconds);
        while (Clock::now() < end) {
            std::this_thread::yield();
        }
    }
    
private:
    using Clock = std::chrono::high_resolution_clock;
    TimePoint m_start;
    TimePoint m_lastFrame;
};

// ============================================================================
// Frame Limiter
// ============================================================================

class FrameLimiter {
public:
    explicit FrameLimiter(f32 targetFPS = 144.0f)
        : m_targetFPS(targetFPS) {
        m_targetFrameTime = 1.0f / targetFPS;
    }
    
    void SetTargetFPS(f32 fps) {
        m_targetFPS = fps;
        m_targetFrameTime = 1.0f / fps;
    }
    
    [[nodiscard]] f32 GetTargetFPS() const { return m_targetFPS; }
    [[nodiscard]] f32 GetTargetFrameTime() const { return m_targetFrameTime; }
    
    void BeginFrame() {
        m_frameStart = Clock::now();
    }
    
    void EndFrame() {
        auto now = Clock::now();
        f32 frameTime = std::chrono::duration<f32>(now - m_frameStart).count();
        
        if (frameTime < m_targetFrameTime) {
            f32 sleepTime = m_targetFrameTime - frameTime;
            Timer::PreciseSleep(sleepTime);
        }
        
        m_lastFrameTime = frameTime;
        m_frameCount++;
    }
    
    [[nodiscard]] f32 GetLastFrameTime() const { return m_lastFrameTime; }
    [[nodiscard]] u64 GetFrameCount() const { return m_frameCount; }
    
private:
    using Clock = std::chrono::high_resolution_clock;
    f32 m_targetFPS;
    f32 m_targetFrameTime;
    f32 m_lastFrameTime = 0.0f;
    u64 m_frameCount = 0;
    TimePoint m_frameStart;
};

// ============================================================================
// Profiler
// ============================================================================

struct ProfileEntry {
    std::string name;
    u64 callCount = 0;
    f64 totalTime = 0.0;      // microseconds
    f64 maxTime = 0.0;
    f64 minTime = std::numeric_limits<f64>::max();
    u32 depth = 0;
    
    [[nodiscard]] f64 AverageTime() const {
        return callCount > 0 ? totalTime / callCount : 0.0;
    }
};

class Profiler {
public:
    static void Initialize();
    static void Shutdown();
    static Profiler& Get();
    
    void BeginFrame();
    void EndFrame();
    
    void PushScope(const char* name);
    void PopScope(const char* name, f64 microseconds);
    
    [[nodiscard]] const std::unordered_map<std::string, ProfileEntry>& GetData() const {
        return m_data;
    }
    
    void ResetStats();
    void DumpStats() const;
    
    [[nodiscard]] f32 GetFrameTime() const { return m_frameTime; }
    [[nodiscard]] f32 GetFPS() const { return m_fps; }
    
    // ImGui visualization
    void DrawUI(bool* open = nullptr);
    
    class ScopeTimer {
    public:
        ScopeTimer(const char* name, Profiler& profiler)
            : m_name(name), m_profiler(profiler), m_start(Clock::now()) {
            m_profiler.PushScope(name);
        }
        
        ~ScopeTimer() {
            auto end = Clock::now();
            f64 elapsed = std::chrono::duration<f64, std::micro>(end - m_start).count();
            m_profiler.PopScope(m_name, elapsed);
        }
        
    private:
        const char* m_name;
        Profiler& m_profiler;
        TimePoint m_start;
    };
    
private:
    using Clock = std::chrono::high_resolution_clock;
    
    std::unordered_map<std::string, ProfileEntry> m_data;
    std::stack<std::string> m_scopeStack;
    mutable std::mutex m_mutex;
    
    TimePoint m_frameStart;
    f32 m_frameTime = 0.0f;
    f32 m_fps = 0.0f;
    u64 m_frameCount = 0;
    f32 m_fpsAccumulator = 0.0f;
};

#define PROFILE_SCOPE(name) auto _prof_##__LINE__ = Duality::Profiler::ScopeTimer(name, Duality::Profiler::Get())
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

// ============================================================================
// Stopwatch
// ============================================================================

class Stopwatch {
public:
    void Start() {
        m_start = Clock::now();
        m_running = true;
    }
    
    void Stop() {
        if (m_running) {
            m_elapsed += std::chrono::duration<f64, std::micro>(Clock::now() - m_start).count();
            m_running = false;
        }
    }
    
    void Reset() {
        m_elapsed = 0.0;
        m_start = Clock::now();
    }
    
    void Restart() {
        Reset();
        Start();
    }
    
    [[nodiscard]] f64 GetMicroseconds() const {
        if (m_running) {
            return m_elapsed + std::chrono::duration<f64, std::micro>(Clock::now() - m_start).count();
        }
        return m_elapsed;
    }
    
    [[nodiscard]] f64 GetMilliseconds() const { return GetMicroseconds() / 1000.0; }
    [[nodiscard]] f64 GetSeconds() const { return GetMicroseconds() / 1000000.0; }
    
private:
    using Clock = std::chrono::high_resolution_clock;
    TimePoint m_start;
    f64 m_elapsed = 0.0;
    bool m_running = false;
};

} // namespace Duality