// =============================================================================
// FILE: src/Engine/Core/Memory.h
// PURPOSE: Custom memory allocators and tracking
// =============================================================================

#pragma once

#include "Types.h"
#include <new>

namespace Duality {

// ============================================================================
// Memory Statistics
// ============================================================================

struct MemoryStats {
    usize totalAllocated = 0;
    usize totalFreed = 0;
    usize currentUsage = 0;
    usize peakUsage = 0;
    u64 allocationCount = 0;
    u64 freeCount = 0;
    
    struct CategoryStats {
        usize allocated = 0;
        u64 count = 0;
    };
    std::unordered_map<std::string, CategoryStats> categories;
    
    void Reset() {
        totalAllocated = 0;
        totalFreed = 0;
        currentUsage = 0;
        peakUsage = 0;
        allocationCount = 0;
        freeCount = 0;
        categories.clear();
    }
};

// ============================================================================
// Memory Tracker (Debug)
// ============================================================================

class MemoryTracker {
public:
    static void Initialize();
    static void Shutdown();
    static MemoryTracker& Get();
    
    void RecordAllocation(void* ptr, usize size, const char* category,
                          const char* file, i32 line);
    void RecordFree(void* ptr);
    
    [[nodiscard]] MemoryStats GetStats() const;
    void DumpStats() const;
    void CheckLeaks() const;
    
private:
    struct AllocationInfo {
        usize size;
        std::string category;
        std::string file;
        i32 line;
        TimePoint time;
    };
    
    std::unordered_map<void*, AllocationInfo> m_allocations;
    mutable std::mutex m_mutex;
    MemoryStats m_stats;
};

// ============================================================================
// Pool Allocator
// ============================================================================

template<typename T, usize BlockSize = 1024>
class PoolAllocator {
public:
    PoolAllocator() = default;
    ~PoolAllocator() { Clear(); }
    
    NON_COPYABLE(PoolAllocator);
    DEFAULT_MOVABLE(PoolAllocator);
    
    T* Allocate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_freeList == nullptr) {
            AllocateBlock();
        }
        
        T* obj = reinterpret_cast<T*>(m_freeList);
        m_freeList = m_freeList->next;
        m_usedCount++;
        
        return new (obj) T();
    }
    
    void Free(T* obj) {
        if (!obj) return;
        
        obj->~T();
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        FreeNode* node = reinterpret_cast<FreeNode*>(obj);
        node->next = m_freeList;
        m_freeList = node;
        m_usedCount--;
    }
    
    template<typename... Args>
    T* Construct(Args&&... args) {
        T* obj = Allocate();
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }
    
    [[nodiscard]] usize GetUsedCount() const { return m_usedCount; }
    [[nodiscard]] usize GetCapacity() const { return m_blocks.size() * BlockSize; }
    
    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto block : m_blocks) {
            std::free(block);
        }
        m_blocks.clear();
        m_freeList = nullptr;
        m_usedCount = 0;
    }
    
private:
    struct FreeNode {
        FreeNode* next = nullptr;
    };
    
    void AllocateBlock() {
        T* block = static_cast<T*>(std::malloc(sizeof(T) * BlockSize));
        if (!block) throw std::bad_alloc();
        
        m_blocks.push_back(block);
        
        // Initialize free list
        for (usize i = 0; i < BlockSize; i++) {
            FreeNode* node = reinterpret_cast<FreeNode*>(&block[i]);
            node->next = m_freeList;
            m_freeList = node;
        }
    }
    
    std::vector<T*> m_blocks;
    FreeNode* m_freeList = nullptr;
    usize m_usedCount = 0;
    std::mutex m_mutex;
};

// ============================================================================
// Arena Allocator
// ============================================================================

class ArenaAllocator {
public:
    explicit ArenaAllocator(usize initialSize = 1 * Constants::MB)
        : m_capacity(initialSize) {
        m_buffer = static_cast<std::byte*>(std::malloc(m_capacity));
        if (!m_buffer) throw std::bad_alloc();
    }
    
    ~ArenaAllocator() {
        std::free(m_buffer);
    }
    
    NON_COPYABLE(ArenaAllocator);
    DEFAULT_MOVABLE(ArenaAllocator);
    
    void* Allocate(usize size, usize alignment = alignof(std::max_align_t)) {
        usize alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);
        
        if (alignedOffset + size > m_capacity) {
            // Grow arena
            Grow(size);
            alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);
        }
        
        void* ptr = m_buffer + alignedOffset;
        m_offset = alignedOffset + size;
        m_highWaterMark = std::max(m_highWaterMark, m_offset);
        
        return ptr;
    }
    
    template<typename T, typename... Args>
    T* Construct(Args&&... args) {
        void* mem = Allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }
    
    void Reset() {
        m_offset = 0;
    }
    
    void Clear() {
        Reset();
        if (m_buffer) {
            std::free(m_buffer);
            m_buffer = nullptr;
        }
        m_capacity = 0;
    }
    
    void Grow(usize minSize) {
        usize newCapacity = m_capacity * 2;
        while (newCapacity < m_offset + minSize) {
            newCapacity *= 2;
        }
        
        std::byte* newBuffer = static_cast<std::byte*>(std::malloc(newCapacity));
        if (!newBuffer) throw std::bad_alloc();
        
        std::memcpy(newBuffer, m_buffer, m_offset);
        std::free(m_buffer);
        
        m_buffer = newBuffer;
        m_capacity = newCapacity;
    }
    
    [[nodiscard]] usize GetUsed() const { return m_offset; }
    [[nodiscard]] usize GetCapacity() const { return m_capacity; }
    [[nodiscard]] usize GetHighWaterMark() const { return m_highWaterMark; }
    
private:
    std::byte* m_buffer = nullptr;
    usize m_capacity = 0;
    usize m_offset = 0;
    usize m_highWaterMark = 0;
};

// ============================================================================
// Linear Allocator
// ============================================================================

class LinearAllocator {
public:
    explicit LinearAllocator(usize size) : m_size(size) {
        m_buffer = static_cast<std::byte*>(std::malloc(m_size));
        if (!m_buffer) throw std::bad_alloc();
    }
    
    ~LinearAllocator() {
        std::free(m_buffer);
    }
    
    NON_COPYABLE(LinearAllocator);
    DEFAULT_MOVABLE(LinearAllocator);
    
    void* Allocate(usize size, usize alignment = alignof(std::max_align_t)) {
        usize alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);
        
        if (alignedOffset + size > m_size) {
            return nullptr;
        }
        
        void* ptr = m_buffer + alignedOffset;
        m_offset = alignedOffset + size;
        
        return ptr;
    }
    
    void Reset() {
        m_offset = 0;
    }
    
    [[nodiscard]] usize GetUsed() const { return m_offset; }
    [[nodiscard]] usize GetRemaining() const { return m_size - m_offset; }
    
private:
    std::byte* m_buffer = nullptr;
    usize m_size = 0;
    usize m_offset = 0;
};

// ============================================================================
// Stack Allocator
// ============================================================================

class StackAllocator {
public:
    explicit StackAllocator(usize size) : m_size(size) {
        m_buffer = static_cast<std::byte*>(std::malloc(m_size));
        if (!m_buffer) throw std::bad_alloc();
    }
    
    ~StackAllocator() {
        std::free(m_buffer);
    }
    
    NON_COPYABLE(StackAllocator);
    DEFAULT_MOVABLE(StackAllocator);
    
    struct Marker {
        usize offset;
    };
    
    void* Allocate(usize size, usize alignment = alignof(std::max_align_t)) {
        usize alignedOffset = (m_offset + alignment - 1) & ~(alignment - 1);
        
        if (alignedOffset + size > m_size) {
            return nullptr;
        }
        
        void* ptr = m_buffer + alignedOffset;
        m_offset = alignedOffset + size;
        
        return ptr;
    }
    
    Marker GetMarker() const {
        return Marker{m_offset};
    }
    
    void FreeToMarker(const Marker& marker) {
        m_offset = marker.offset;
    }
    
    void Reset() {
        m_offset = 0;
    }
    
    [[nodiscard]] usize GetUsed() const { return m_offset; }
    [[nodiscard]] usize GetRemaining() const { return m_size - m_offset; }
    
private:
    std::byte* m_buffer = nullptr;
    usize m_size = 0;
    usize m_offset = 0;
};

// ============================================================================
// Aligned Allocation
// ============================================================================

inline void* AlignedAlloc(usize size, usize alignment) {
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    return std::aligned_alloc(alignment, size);
#endif
}

inline void AlignedFree(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

// ============================================================================
// Smart Pointer Aliases
// ============================================================================

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename... Args>
Ref<T> MakeRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
Scope<T> MakeScope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// ============================================================================
// New/Delete Overrides (Debug)
// ============================================================================

#ifdef DUALITY_DEBUG_MEMORY

void* operator new(usize size);
void* operator new[](usize size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

#endif

} // namespace Duality