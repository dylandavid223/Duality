// =============================================================================
// FILE: src/Engine/Core/Memory.cpp
// PURPOSE: Memory tracking implementation
// =============================================================================

#include "Memory.h"

#ifdef DUALITY_DEBUG_MEMORY

namespace Duality {

// Global memory tracker instance
static MemoryTracker* g_tracker = nullptr;

void* operator new(usize size) {
    void* ptr = std::malloc(size);
    if (g_tracker) {
        g_tracker->RecordAllocation(ptr, size, "global", __FILE__, __LINE__);
    }
    return ptr;
}

void* operator new[](usize size) {
    void* ptr = std::malloc(size);
    if (g_tracker) {
        g_tracker->RecordAllocation(ptr, size, "global[]", __FILE__, __LINE__);
    }
    return ptr;
}

void operator delete(void* ptr) noexcept {
    if (g_tracker) {
        g_tracker->RecordFree(ptr);
    }
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept {
    if (g_tracker) {
        g_tracker->RecordFree(ptr);
    }
    std::free(ptr);
}

void MemoryTracker::Initialize() {
    if (!g_tracker) {
        g_tracker = new MemoryTracker();
    }
}

void MemoryTracker::Shutdown() {
    if (g_tracker) {
        g_tracker->CheckLeaks();
        delete g_tracker;
        g_tracker = nullptr;
    }
}

MemoryTracker& MemoryTracker::Get() {
    return *g_tracker;
}

void MemoryTracker::RecordAllocation(void* ptr, usize size, const char* category,
                                      const char* file, i32 line) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    AllocationInfo info{
        .size = size,
        .category = category,
        .file = file,
        .line = line,
        .time = std::chrono::high_resolution_clock::now()
    };
    
    m_allocations[ptr] = info;
    
    m_stats.totalAllocated += size;
    m_stats.currentUsage += size;
    m_stats.allocationCount++;
    
    if (m_stats.currentUsage > m_stats.peakUsage) {
        m_stats.peakUsage = m_stats.currentUsage;
    }
    
    auto& catStats = m_stats.categories[category];
    catStats.allocated += size;
    catStats.count++;
}

void MemoryTracker::RecordFree(void* ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        m_stats.totalFreed += it->second.size;
        m_stats.currentUsage -= it->second.size;
        m_stats.freeCount++;
        m_allocations.erase(it);
    }
}

MemoryStats MemoryTracker::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stats;
}

void MemoryTracker::DumpStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    LOG_INFO("=== Memory Statistics ===");
    LOG_INFO("Total Allocated: {:.2f} MB", m_stats.totalAllocated / (f32)Constants::MB);
    LOG_INFO("Total Freed: {:.2f} MB", m_stats.totalFreed / (f32)Constants::MB);
    LOG_INFO("Current Usage: {:.2f} MB", m_stats.currentUsage / (f32)Constants::MB);
    LOG_INFO("Peak Usage: {:.2f} MB", m_stats.peakUsage / (f32)Constants::MB);
    LOG_INFO("Allocations: {}", m_stats.allocationCount);
    LOG_INFO("Frees: {}", m_stats.freeCount);
    LOG_INFO("Leaks: {}", m_stats.allocationCount - m_stats.freeCount);
    
    LOG_INFO("--- Categories ---");
    for (const auto& [category, stats] : m_stats.categories) {
        LOG_INFO("  {}: {:.2f} MB ({} allocations)", 
                 category, stats.allocated / (f32)Constants::MB, stats.count);
    }
}

void MemoryTracker::CheckLeaks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_allocations.empty()) {
        LOG_ERROR("Memory leaks detected: {} allocations not freed", m_allocations.size());
        
        for (const auto& [ptr, info] : m_allocations) {
            LOG_ERROR("  Leak at {}: {} bytes from {}:{}",
                      ptr, info.size, info.file, info.line);
        }
    } else {
        LOG_INFO("No memory leaks detected");
    }
}

} // namespace Duality

#else

namespace Duality {

void MemoryTracker::Initialize() {}
void MemoryTracker::Shutdown() {}
MemoryTracker& MemoryTracker::Get() { static MemoryTracker tracker; return tracker; }
void MemoryTracker::RecordAllocation(void*, usize, const char*, const char*, i32) {}
void MemoryTracker::RecordFree(void*) {}
MemoryStats MemoryTracker::GetStats() const { return {}; }
void MemoryTracker::DumpStats() const {}
void MemoryTracker::CheckLeaks() const {}

} // namespace Duality

#endif