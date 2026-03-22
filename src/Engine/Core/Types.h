// =============================================================================
// FILE: src/Engine/Core/Types.h
// PURPOSE: Fundamental type definitions and utilities
// =============================================================================

#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <limits>
#include <utility>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <expected>
#include <span>
#include <chrono>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <format>

namespace Duality {

// ============================================================================
// Type Aliases
// ============================================================================

// Integer types
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Floating point
using f32 = float;
using f64 = double;

// Boolean
using b8  = bool;

// Size type
using usize = size_t;

// ============================================================================
// Constants
// ============================================================================

namespace Constants {
    inline constexpr f32 PI = 3.14159265358979323846f;
    inline constexpr f32 TWO_PI = 6.28318530717958647692f;
    inline constexpr f32 HALF_PI = 1.57079632679489661923f;
    inline constexpr f32 INV_PI = 0.31830988618379067153f;
    inline constexpr f32 DEG_TO_RAD = PI / 180.0f;
    inline constexpr f32 RAD_TO_DEG = 180.0f / PI;
    
    inline constexpr f32 EPSILON = 1e-6f;
    inline constexpr f32 SMALL_EPSILON = 1e-4f;
    inline constexpr f32 LARGE_EPSILON = 1e-2f;
    
    inline constexpr f32 INFINITY_F32 = std::numeric_limits<f32>::infinity();
    inline constexpr f32 NEG_INFINITY_F32 = -std::numeric_limits<f32>::infinity();
    inline constexpr f32 MAX_F32 = std::numeric_limits<f32>::max();
    inline constexpr f32 MIN_F32 = std::numeric_limits<f32>::lowest();
    
    inline constexpr i32 MAX_I32 = std::numeric_limits<i32>::max();
    inline constexpr i32 MIN_I32 = std::numeric_limits<i32>::lowest();
    
    inline constexpr u64 MAX_U64 = std::numeric_limits<u64>::max();
    
    inline constexpr usize KB = 1024;
    inline constexpr usize MB = 1024 * KB;
    inline constexpr usize GB = 1024 * MB;
}

// ============================================================================
// Time Types
// ============================================================================

using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::high_resolution_clock::duration;
using Seconds = std::chrono::duration<f32>;
using Milliseconds = std::chrono::duration<f32, std::milli>;
using Microseconds = std::chrono::duration<f32, std::micro>;

// ============================================================================
// Result Type
// ============================================================================

template<typename T, typename E = std::string>
using Result = std::expected<T, E>;

// ============================================================================
// Handle Types
// ============================================================================

using ResourceHandle = u64;
using EntityHandle = u32;
using ComponentHandle = u32;

inline constexpr ResourceHandle INVALID_RESOURCE_HANDLE = Constants::MAX_U64;
inline constexpr EntityHandle INVALID_ENTITY_HANDLE = Constants::MAX_I32;
inline constexpr ComponentHandle INVALID_COMPONENT_HANDLE = Constants::MAX_I32;

// ============================================================================
// Version Info
// ============================================================================

struct Version {
    u32 major = 1;
    u32 minor = 0;
    u32 patch = 0;
    u32 build = 0;
    
    [[nodiscard]] std::string ToString() const {
        return std::format("{}.{}.{}.{}", major, minor, patch, build);
    }
    
    [[nodiscard]] bool operator==(const Version& other) const = default;
    [[nodiscard]] bool operator<(const Version& other) const {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        if (patch != other.patch) return patch < other.patch;
        return build < other.build;
    }
};

inline constexpr Version ENGINE_VERSION{1, 0, 0, 0};

// ============================================================================
// Utility Functions
// ============================================================================

// Alignment
template<typename T>
inline constexpr T AlignUp(T value, T alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

template<typename T>
inline constexpr T AlignDown(T value, T alignment) {
    return value & ~(alignment - 1);
}

template<typename T>
inline constexpr bool IsPowerOfTwo(T value) {
    return value && !(value & (value - 1));
}

// Byte manipulation
inline constexpr u32 MakeFourCC(char a, char b, char c, char d) {
    return (static_cast<u32>(a) << 0)  |
           (static_cast<u32>(b) << 8)  |
           (static_cast<u32>(c) << 16) |
           (static_cast<u32>(d) << 24);
}

// Defer pattern
template<typename F>
struct Defer {
    F f;
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
};

#define DEFER(code) auto _defer_##__LINE__ = Defer([&](){ code; })

// Non-copyable/movable macros
#define NON_COPYABLE(Type) \
    Type(const Type&) = delete; \
    Type& operator=(const Type&) = delete;

#define NON_MOVABLE(Type) \
    Type(Type&&) = delete; \
    Type& operator=(Type&&) = delete;

#define DEFAULT_COPYABLE(Type) \
    Type(const Type&) = default; \
    Type& operator=(const Type&) = default;

#define DEFAULT_MOVABLE(Type) \
    Type(Type&&) = default; \
    Type& operator=(Type&&) = default;

// ============================================================================
// Hash Functions
// ============================================================================

// FNV-1a hash
inline constexpr u64 HashFNV1a(const char* str, u64 hash = 14695981039346656037ULL) {
    return *str ? HashFNV1a(str + 1, (hash ^ static_cast<u64>(*str)) * 1099511628211ULL) : hash;
}

inline constexpr u64 operator"" _hash(const char* str, size_t) {
    return HashFNV1a(str);
}

template<typename T>
struct Hash {
    usize operator()(const T& value) const {
        return std::hash<T>{}(value);
    }
};

// ============================================================================
// Safe Casting
// ============================================================================

template<typename To, typename From>
inline constexpr To NarrowCast(From value) {
    static_assert(std::is_arithmetic_v<From> && std::is_arithmetic_v<To>);
    To result = static_cast<To>(value);
    assert(static_cast<From>(result) == value);
    return result;
}

// ============================================================================
// Span Helpers
// ============================================================================

template<typename T>
inline constexpr std::span<const std::byte> AsBytes(const T& value) {
    return std::span<const std::byte>(reinterpret_cast<const std::byte*>(&value), sizeof(T));
}

template<typename T>
inline constexpr std::span<std::byte> AsBytes(T& value) {
    return std::span<std::byte>(reinterpret_cast<std::byte*>(&value), sizeof(T));
}

template<typename T>
inline constexpr std::span<const T> AsSpan(const std::vector<T>& vec) {
    return std::span<const T>(vec.data(), vec.size());
}

template<typename T>
inline constexpr std::span<T> AsSpan(std::vector<T>& vec) {
    return std::span<T>(vec.data(), vec.size());
}

// ============================================================================
// Scope Guards
// ============================================================================

template<typename F>
class ScopeGuard {
public:
    explicit ScopeGuard(F&& func) : m_func(std::forward<F>(func)), m_active(true) {}
    ~ScopeGuard() { if (m_active) m_func(); }
    void Dismiss() { m_active = false; }
    
    ScopeGuard(ScopeGuard&& other) noexcept 
        : m_func(std::move(other.m_func)), m_active(other.m_active) {
        other.Dismiss();
    }
    
    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            if (m_active) m_func();
            m_func = std::move(other.m_func);
            m_active = other.m_active;
            other.Dismiss();
        }
        return *this;
    }
    
private:
    F m_func;
    bool m_active;
};

template<typename F>
ScopeGuard(F&&) -> ScopeGuard<std::decay_t<F>>;

// ============================================================================
// Bit Operations
// ============================================================================

template<typename T>
inline constexpr bool HasFlag(T value, T flag) {
    return (value & flag) == flag;
}

template<typename T>
inline constexpr T SetFlag(T value, T flag) {
    return value | flag;
}

template<typename T>
inline constexpr T ClearFlag(T value, T flag) {
    return value & ~flag;
}

template<typename T>
inline constexpr T ToggleFlag(T value, T flag) {
    return value ^ flag;
}

// ============================================================================
// Color Utilities
// ============================================================================

struct Color {
    u8 r, g, b, a;
    
    constexpr Color() : r(0), g(0), b(0), a(255) {}
    constexpr Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}
    
    [[nodiscard]] u32 ToRGBA() const {
        return (r << 24) | (g << 16) | (b << 8) | a;
    }
    
    [[nodiscard]] u32 ToBGRA() const {
        return (b << 24) | (g << 16) | (r << 8) | a;
    }
    
    [[nodiscard]] Vec3 ToVec3() const {
        return Vec3(r / 255.0f, g / 255.0f, b / 255.0f);
    }
    
    [[nodiscard]] Vec4 ToVec4() const {
        return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
    
    // Common colors
    static constexpr Color BLACK() { return Color(0, 0, 0, 255); }
    static constexpr Color WHITE() { return Color(255, 255, 255, 255); }
    static constexpr Color RED() { return Color(255, 0, 0, 255); }
    static constexpr Color GREEN() { return Color(0, 255, 0, 255); }
    static constexpr Color BLUE() { return Color(0, 0, 255, 255); }
    static constexpr Color YELLOW() { return Color(255, 255, 0, 255); }
    static constexpr Color CYAN() { return Color(0, 255, 255, 255); }
    static constexpr Color MAGENTA() { return Color(255, 0, 255, 255); }
    static constexpr Color ORANGE() { return Color(255, 165, 0, 255); }
    static constexpr Color PURPLE() { return Color(128, 0, 128, 255); }
    static constexpr Color GRAY() { return Color(128, 128, 128, 255); }
    static constexpr Color DARK_GRAY() { return Color(64, 64, 64, 255); }
    static constexpr Color LIGHT_GRAY() { return Color(192, 192, 192, 255); }
};

// ============================================================================
// String Utilities
// ============================================================================

inline std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

inline std::string TrimString(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

inline std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline std::string ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// ============================================================================
// File Utilities
// ============================================================================

inline std::vector<u8> ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    usize size = file.tellg();
    std::vector<u8> buffer(size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

inline bool WriteFile(const std::string& filename, const std::vector<u8>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

inline std::string GetExtension(const std::string& filename) {
    usize pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos + 1);
}

inline std::string GetFileName(const std::string& path) {
    usize pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

inline std::string GetDirectory(const std::string& path) {
    usize pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    return path.substr(0, pos);
}

} // namespace Duality