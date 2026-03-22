// =============================================================================
// FILE: src/Engine/Math/Math.h
// PURPOSE: Complete math library for game development
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace Duality {

// ============================================================================
// Constants
// ============================================================================

namespace Math {
    inline constexpr f32 PI = 3.14159265358979323846f;
    inline constexpr f32 TWO_PI = 6.28318530717958647692f;
    inline constexpr f32 HALF_PI = 1.57079632679489661923f;
    inline constexpr f32 QUARTER_PI = 0.78539816339744830962f;
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
}

// ============================================================================
// Vector 2
// ============================================================================

struct Vec2 {
    f32 x, y;
    
    // Constructors
    constexpr Vec2() : x(0), y(0) {}
    constexpr Vec2(f32 x, f32 y) : x(x), y(y) {}
    explicit constexpr Vec2(f32 v) : x(v), y(v) {}
    
    // Array access
    f32& operator[](usize i) { return (&x)[i]; }
    const f32& operator[](usize i) const { return (&x)[i]; }
    
    // Arithmetic operators
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(const Vec2& o) const { return Vec2(x * o.x, y * o.y); }
    Vec2 operator/(const Vec2& o) const { return Vec2(x / o.x, y / o.y); }
    Vec2 operator*(f32 s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(f32 s) const { return Vec2(x / s, y / s); }
    Vec2 operator-() const { return Vec2(-x, -y); }
    
    // Assignment operators
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(f32 s) { x *= s; y *= s; return *this; }
    Vec2& operator/=(f32 s) { x /= s; y /= s; return *this; }
    
    // Comparison
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
    
    // Vector operations
    [[nodiscard]] f32 Length() const { return std::sqrt(x * x + y * y); }
    [[nodiscard]] f32 LengthSq() const { return x * x + y * y; }
    [[nodiscard]] Vec2 Normalized() const {
        f32 len = Length();
        return len > Math::EPSILON ? *this / len : Vec2(0);
    }
    void Normalize() { *this = Normalized(); }
    
    [[nodiscard]] f32 Dot(const Vec2& o) const { return x * o.x + y * o.y; }
    [[nodiscard]] f32 Cross(const Vec2& o) const { return x * o.y - y * o.x; }
    
    [[nodiscard]] f32 Distance(const Vec2& o) const { return (*this - o).Length(); }
    [[nodiscard]] f32 DistanceSq(const Vec2& o) const { return (*this - o).LengthSq(); }
    
    [[nodiscard]] Vec2 Perpendicular() const { return Vec2(-y, x); }
    [[nodiscard]] Vec2 Reflect(const Vec2& normal) const {
        return *this - normal * (2.0f * Dot(normal));
    }
    
    [[nodiscard]] Vec2 Lerp(const Vec2& target, f32 t) const {
        return *this + (target - *this) * std::clamp(t, 0.0f, 1.0f);
    }
    
    [[nodiscard]] bool IsZero() const { return std::abs(x) < Math::EPSILON && std::abs(y) < Math::EPSILON; }
    [[nodiscard]] bool IsNormalized() const { return std::abs(LengthSq() - 1.0f) < Math::EPSILON; }
    
    // Constants
    static constexpr Vec2 Zero() { return Vec2(0, 0); }
    static constexpr Vec2 One() { return Vec2(1, 1); }
    static constexpr Vec2 Right() { return Vec2(1, 0); }
    static constexpr Vec2 Left() { return Vec2(-1, 0); }
    static constexpr Vec2 Up() { return Vec2(0, 1); }
    static constexpr Vec2 Down() { return Vec2(0, -1); }
};

// ============================================================================
// Vector 3
// ============================================================================

struct Vec3 {
    f32 x, y, z;
    
    // Constructors
    constexpr Vec3() : x(0), y(0), z(0) {}
    constexpr Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
    explicit constexpr Vec3(f32 v) : x(v), y(v), z(v) {}
    constexpr Vec3(const Vec2& v, f32 z) : x(v.x), y(v.y), z(z) {}
    
    // Array access
    f32& operator[](usize i) { return (&x)[i]; }
    const f32& operator[](usize i) const { return (&x)[i]; }
    
    // Arithmetic operators
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(const Vec3& o) const { return Vec3(x * o.x, y * o.y, z * o.z); }
    Vec3 operator/(const Vec3& o) const { return Vec3(x / o.x, y / o.y, z / o.z); }
    Vec3 operator*(f32 s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(f32 s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    
    // Assignment operators
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(f32 s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(f32 s) { x /= s; y /= s; z /= s; return *this; }
    
    // Comparison
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const Vec3& o) const { return !(*this == o); }
    
    // Vector operations
    [[nodiscard]] f32 Length() const { return std::sqrt(x * x + y * y + z * z); }
    [[nodiscard]] f32 LengthSq() const { return x * x + y * y + z * z; }
    [[nodiscard]] Vec3 Normalized() const {
        f32 len = Length();
        return len > Math::EPSILON ? *this / len : Vec3(0);
    }
    void Normalize() { *this = Normalized(); }
    
    [[nodiscard]] f32 Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    [[nodiscard]] Vec3 Cross(const Vec3& o) const {
        return Vec3(
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        );
    }
    
    [[nodiscard]] f32 Distance(const Vec3& o) const { return (*this - o).Length(); }
    [[nodiscard]] f32 DistanceSq(const Vec3& o) const { return (*this - o).LengthSq(); }
    
    [[nodiscard]] Vec3 Reflect(const Vec3& normal) const {
        return *this - normal * (2.0f * Dot(normal));
    }
    
    [[nodiscard]] Vec3 Refract(const Vec3& normal, f32 eta) const {
        f32 ndot = Dot(normal);
        f32 k = 1.0f - eta * eta * (1.0f - ndot * ndot);
        return k < 0.0f ? Vec3(0) : (*this - normal * ndot) * eta - normal * std::sqrt(k);
    }
    
    [[nodiscard]] Vec3 Project(const Vec3& onto) const {
        f32 dot = Dot(onto);
        f32 lenSq = onto.LengthSq();
        return lenSq > Math::EPSILON ? onto * (dot / lenSq) : Vec3(0);
    }
    
    [[nodiscard]] Vec3 Reject(const Vec3& from) const {
        return *this - Project(from);
    }
    
    [[nodiscard]] Vec3 Lerp(const Vec3& target, f32 t) const {
        return *this + (target - *this) * std::clamp(t, 0.0f, 1.0f);
    }
    
    [[nodiscard]] Vec3 Slerp(const Vec3& target, f32 t) const {
        f32 dot = Dot(target);
        dot = std::clamp(dot, -1.0f, 1.0f);
        
        if (dot > 0.9995f) {
            return Lerp(target, t).Normalized();
        }
        
        f32 theta0 = std::acos(dot);
        f32 theta = theta0 * t;
        
        Vec3 v2 = target - *this * dot;
        v2.Normalize();
        
        return *this * std::cos(theta) + v2 * std::sin(theta);
    }
    
    [[nodiscard]] Vec3 RotateAround(const Vec3& axis, f32 angle) const {
        Quat q = Quat::FromAxisAngle(axis, angle);
        return q * *this;
    }
    
    [[nodiscard]] bool IsZero() const {
        return std::abs(x) < Math::EPSILON && std::abs(y) < Math::EPSILON && std::abs(z) < Math::EPSILON;
    }
    
    [[nodiscard]] Vec2 XY() const { return Vec2(x, y); }
    [[nodiscard]] Vec2 XZ() const { return Vec2(x, z); }
    [[nodiscard]] Vec2 YZ() const { return Vec2(y, z); }
    
    // Constants
    static constexpr Vec3 Zero() { return Vec3(0, 0, 0); }
    static constexpr Vec3 One() { return Vec3(1, 1, 1); }
    static constexpr Vec3 Right() { return Vec3(1, 0, 0); }
    static constexpr Vec3 Left() { return Vec3(-1, 0, 0); }
    static constexpr Vec3 Up() { return Vec3(0, 1, 0); }
    static constexpr Vec3 Down() { return Vec3(0, -1, 0); }
    static constexpr Vec3 Forward() { return Vec3(0, 0, 1); }
    static constexpr Vec3 Back() { return Vec3(0, 0, -1); }
};

// ============================================================================
// Vector 4
// ============================================================================

struct Vec4 {
    f32 x, y, z, w;
    
    // Constructors
    constexpr Vec4() : x(0), y(0), z(0), w(0) {}
    constexpr Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    explicit constexpr Vec4(f32 v) : x(v), y(v), z(v), w(v) {}
    constexpr Vec4(const Vec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}
    constexpr Vec4(const Vec2& v, f32 z, f32 w) : x(v.x), y(v.y), z(z), w(w) {}
    
    // Array access
    f32& operator[](usize i) { return (&x)[i]; }
    const f32& operator[](usize i) const { return (&x)[i]; }
    
    // Arithmetic operators
    Vec4 operator+(const Vec4& o) const { return Vec4(x + o.x, y + o.y, z + o.z, w + o.w); }
    Vec4 operator-(const Vec4& o) const { return Vec4(x - o.x, y - o.y, z - o.z, w - o.w); }
    Vec4 operator*(const Vec4& o) const { return Vec4(x * o.x, y * o.y, z * o.z, w * o.w); }
    Vec4 operator*(f32 s) const { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4 operator/(f32 s) const { return Vec4(x / s, y / s, z / s, w / s); }
    
    // Assignment operators
    Vec4& operator+=(const Vec4& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    Vec4& operator-=(const Vec4& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
    Vec4& operator*=(f32 s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vec4& operator/=(f32 s) { x /= s; y /= s; z /= s; w /= s; return *this; }
    
    // Vector operations
    [[nodiscard]] f32 Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    [[nodiscard]] f32 LengthSq() const { return x * x + y * y + z * z + w * w; }
    [[nodiscard]] Vec4 Normalized() const {
        f32 len = Length();
        return len > Math::EPSILON ? *this / len : Vec4(0);
    }
    void Normalize() { *this = Normalized(); }
    
    [[nodiscard]] f32 Dot(const Vec4& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
    
    [[nodiscard]] Vec4 Lerp(const Vec4& target, f32 t) const {
        return *this + (target - *this) * std::clamp(t, 0.0f, 1.0f);
    }
    
    [[nodiscard]] Vec3 XYZ() const { return Vec3(x, y, z); }
    
    // Constants
    static constexpr Vec4 Zero() { return Vec4(0, 0, 0, 0); }
    static constexpr Vec4 One() { return Vec4(1, 1, 1, 1); }
};

// ============================================================================
// Quaternion
// ============================================================================

struct Quat {
    f32 x, y, z, w;
    
    // Constructors
    constexpr Quat() : x(0), y(0), z(0), w(1) {}
    constexpr Quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    
    // Factory methods
    static Quat Identity() { return Quat(0, 0, 0, 1); }
    
    static Quat FromAxisAngle(const Vec3& axis, f32 angle) {
        f32 halfAngle = angle * 0.5f;
        f32 s = std::sin(halfAngle);
        Vec3 a = axis.Normalized();
        return Quat(a.x * s, a.y * s, a.z * s, std::cos(halfAngle));
    }
    
    static Quat FromEuler(f32 pitch, f32 yaw, f32 roll) {
        f32 cy = std::cos(yaw * 0.5f);
        f32 sy = std::sin(yaw * 0.5f);
        f32 cp = std::cos(pitch * 0.5f);
        f32 sp = std::sin(pitch * 0.5f);
        f32 cr = std::cos(roll * 0.5f);
        f32 sr = std::sin(roll * 0.5f);
        
        return Quat(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
    }
    
    static Quat FromToRotation(const Vec3& from, const Vec3& to) {
        Vec3 f = from.Normalized();
        Vec3 t = to.Normalized();
        f32 dot = f.Dot(t);
        
        if (dot > 0.999999f) {
            return Identity();
        }
        
        if (dot < -0.999999f) {
            Vec3 axis = Vec3::Right().Cross(f);
            if (axis.LengthSq() < Math::EPSILON) {
                axis = Vec3::Up().Cross(f);
            }
            axis.Normalize();
            return Quat(axis.x, axis.y, axis.z, 0);
        }
        
        Vec3 axis = f.Cross(t);
        axis.Normalize();
        f32 angle = std::acos(dot);
        return FromAxisAngle(axis, angle);
    }
    
    static Quat LookRotation(const Vec3& forward, const Vec3& up = Vec3::Up()) {
        Vec3 f = forward.Normalized();
        Vec3 u = up.Normalized();
        Vec3 r = u.Cross(f).Normalized();
        u = f.Cross(r);
        
        f32 m00 = r.x, m01 = r.y, m02 = r.z;
        f32 m10 = u.x, m11 = u.y, m12 = u.z;
        f32 m20 = f.x, m21 = f.y, m22 = f.z;
        
        f32 tr = m00 + m11 + m22;
        
        if (tr > 0) {
            f32 S = std::sqrt(tr + 1.0f) * 2.0f;
            return Quat(
                (m21 - m12) / S,
                (m02 - m20) / S,
                (m10 - m01) / S,
                0.25f * S
            );
        } else if ((m00 > m11) && (m00 > m22)) {
            f32 S = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
            return Quat(
                0.25f * S,
                (m01 + m10) / S,
                (m02 + m20) / S,
                (m12 - m21) / S
            );
        } else if (m11 > m22) {
            f32 S = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
            return Quat(
                (m01 + m10) / S,
                0.25f * S,
                (m12 + m21) / S,
                (m02 - m20) / S
            );
        } else {
            f32 S = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
            return Quat(
                (m02 + m20) / S,
                (m12 + m21) / S,
                0.25f * S,
                (m01 - m10) / S
            );
        }
    }
    
    // Operations
    Quat operator*(const Quat& o) const {
        return Quat(
            w * o.x + x * o.w + y * o.z - z * o.y,
            w * o.y - x * o.z + y * o.w + z * o.x,
            w * o.z + x * o.y - y * o.x + z * o.w,
            w * o.w - x * o.x - y * o.y - z * o.z
        );
    }
    
    Vec3 operator*(const Vec3& v) const {
        Vec3 qv(x, y, z);
        Vec3 uv = qv.Cross(v);
        Vec3 uuv = qv.Cross(uv);
        return v + uv * (2.0f * w) + uuv * 2.0f;
    }
    
    Quat& operator*=(const Quat& o) {
        *this = *this * o;
        return *this;
    }
    
    [[nodiscard]] Quat Normalized() const {
        f32 len = std::sqrt(x * x + y * y + z * z + w * w);
        return len > Math::EPSILON ? Quat(x / len, y / len, z / len, w / len) : Identity();
    }
    
    void Normalize() { *this = Normalized(); }
    
    [[nodiscard]] Quat Conjugate() const { return Quat(-x, -y, -z, w); }
    [[nodiscard]] Quat Inverse() const { return Conjugate().Normalized(); }
    
    [[nodiscard]] f32 Dot(const Quat& o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
    
    [[nodiscard]] Quat Slerp(const Quat& target, f32 t) const {
        Quat to = target;
        f32 dot = Dot(target);
        
        if (dot < 0.0f) {
            dot = -dot;
            to = Quat(-to.x, -to.y, -to.z, -to.w);
        }
        
        if (dot > 0.9995f) {
            Quat result = *this + (to - *this) * t;
            result.Normalize();
            return result;
        }
        
        f32 theta0 = std::acos(dot);
        f32 theta = theta0 * t;
        f32 sinTheta = std::sin(theta);
        f32 sinTheta0 = std::sin(theta0);
        
        f32 s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
        f32 s1 = sinTheta / sinTheta0;
        
        Quat result = *this * s0 + to * s1;
        result.Normalize();
        return result;
    }
    
    [[nodiscard]] Quat NLerp(const Quat& target, f32 t) const {
        Quat result = *this + (target - *this) * t;
        result.Normalize();
        return result;
    }
    
    [[nodiscard]] Vec3 GetAxis() const {
        f32 s = std::sqrt(1.0f - w * w);
        return s < Math::EPSILON ? Vec3::Up() : Vec3(x, y, z) / s;
    }
    
    [[nodiscard]] f32 GetAngle() const {
        return 2.0f * std::acos(std::clamp(w, -1.0f, 1.0f));
    }
    
    [[nodiscard]] Vec3 ToEuler() const {
        Vec3 euler;
        
        // Pitch (x-axis)
        f32 sinr_cosp = 2.0f * (w * x + y * z);
        f32 cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        euler.x = std::atan2(sinr_cosp, cosr_cosp);
        
        // Yaw (y-axis)
        f32 sinp = 2.0f * (w * y - z * x);
        if (std::abs(sinp) >= 1.0f)
            euler.y = std::copysign(Math::HALF_PI, sinp);
        else
            euler.y = std::asin(sinp);
        
        // Roll (z-axis)
        f32 siny_cosp = 2.0f * (w * z + x * y);
        f32 cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        euler.z = std::atan2(siny_cosp, cosy_cosp);
        
        return euler;
    }
    
    [[nodiscard]] Mat4 ToMatrix() const;
};

// ============================================================================
// Matrix 4x4
// ============================================================================

struct alignas(16) Mat4 {
    union {
        f32 m[16];
        struct {
            f32 m00, m01, m02, m03;
            f32 m10, m11, m12, m13;
            f32 m20, m21, m22, m23;
            f32 m30, m31, m32, m33;
        };
        Vec4 rows[4];
    };
    
    // Constructors
    constexpr Mat4() : m00(1), m01(0), m02(0), m03(0),
                       m10(0), m11(1), m12(0), m13(0),
                       m20(0), m21(0), m22(1), m23(0),
                       m30(0), m31(0), m32(0), m33(1) {}
    
    explicit constexpr Mat4(f32 diagonal) : m00(diagonal), m01(0), m02(0), m03(0),
                                            m10(0), m11(diagonal), m12(0), m13(0),
                                            m20(0), m21(0), m22(diagonal), m23(0),
                                            m30(0), m31(0), m32(0), m33(diagonal) {}
    
    constexpr Mat4(
        f32 m00, f32 m01, f32 m02, f32 m03,
        f32 m10, f32 m11, f32 m12, f32 m13,
        f32 m20, f32 m21, f32 m22, f32 m23,
        f32 m30, f32 m31, f32 m32, f32 m33
    ) : m00(m00), m01(m01), m02(m02), m03(m03),
        m10(m10), m11(m11), m12(m12), m13(m13),
        m20(m20), m21(m21), m22(m22), m23(m23),
        m30(m30), m31(m31), m32(m32), m33(m33) {}
    
    // Access
    f32* operator[](usize row) { return &rows[row].x; }
    const f32* operator[](usize row) const { return &rows[row].x; }
    
    Vec4 Row(usize i) const { return rows[i]; }
    Vec4 Column(usize i) const {
        return Vec4(rows[0][i], rows[1][i], rows[2][i], rows[3][i]);
    }
    
    // Arithmetic
    Mat4 operator*(const Mat4& o) const {
        Mat4 result;
        for (i32 i = 0; i < 4; i++) {
            for (i32 j = 0; j < 4; j++) {
                result.m[i * 4 + j] = 
                    m[i * 4 + 0] * o.m[0 * 4 + j] +
                    m[i * 4 + 1] * o.m[1 * 4 + j] +
                    m[i * 4 + 2] * o.m[2 * 4 + j] +
                    m[i * 4 + 3] * o.m[3 * 4 + j];
            }
        }
        return result;
    }
    
    Vec4 operator*(const Vec4& v) const {
        return Vec4(
            rows[0].Dot(v),
            rows[1].Dot(v),
            rows[2].Dot(v),
            rows[3].Dot(v)
        );
    }
    
    Mat4& operator*=(const Mat4& o) {
        *this = *this * o;
        return *this;
    }
    
    // Transform operations
    Vec3 TransformPoint(const Vec3& p) const {
        Vec4 result = *this * Vec4(p, 1.0f);
        return result.XYZ() / result.w;
    }
    
    Vec3 TransformVector(const Vec3& v) const {
        return (*this * Vec4(v, 0.0f)).XYZ();
    }
    
    Vec3 TransformNormal(const Vec3& n) const {
        return Transposed().Inverted().TransformVector(n).Normalized();
    }
    
    // Matrix operations
    [[nodiscard]] Mat4 Transposed() const {
        return Mat4(
            m00, m10, m20, m30,
            m01, m11, m21, m31,
            m02, m12, m22, m32,
            m03, m13, m23, m33
        );
    }
    
    [[nodiscard]] Mat4 Inverted() const {
        f32 inv[16];
        
        inv[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6]  * m[15] + 
                 m[9]  * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];
        inv[1] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6]  * m[15] - 
                  m[8]  * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];
        inv[2] = m[4]  * m[9]  * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5]  * m[15] + 
                 m[8]  * m[7]  * m[13] + m[12] * m[5]  * m[11] - m[12] * m[7]  * m[9];
        inv[3] = -m[4]  * m[9]  * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5]  * m[14] - 
                  m[8]  * m[6]  * m[13] - m[12] * m[5]  * m[10] + m[12] * m[6]  * m[9];
        
        inv[4] = -m[1]  * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2]  * m[15] - 
                  m[9]  * m[3]  * m[14] - m[13] * m[2]  * m[11] + m[13] * m[3]  * m[10];
        inv[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2]  * m[15] + 
                 m[8]  * m[3]  * m[14] + m[12] * m[2]  * m[11] - m[12] * m[3]  * m[10];
        inv[6] = -m[0]  * m[9]  * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1]  * m[15] - 
                  m[8]  * m[3]  * m[13] - m[12] * m[1]  * m[11] + m[12] * m[3]  * m[9];
        inv[7] = m[0]  * m[9]  * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1]  * m[14] + 
                 m[8]  * m[2]  * m[13] + m[12] * m[1]  * m[10] - m[12] * m[2]  * m[9];
        
        inv[8] = m[1]  * m[6]  * m[15] - m[1]  * m[7]  * m[14] - m[5]  * m[2]  * m[15] + 
                 m[5]  * m[3]  * m[14] + m[13] * m[2]  * m[7]  - m[13] * m[3]  * m[6];
        inv[9] = -m[0]  * m[6]  * m[15] + m[0]  * m[7]  * m[14] + m[4]  * m[2]  * m[15] - 
                  m[4]  * m[3]  * m[14] - m[12] * m[2]  * m[7]  + m[12] * m[3]  * m[6];
        inv[10] = m[0]  * m[5]  * m[15] - m[0]  * m[7]  * m[13] - m[4]  * m[1]  * m[15] + 
                  m[4]  * m[3]  * m[13] + m[12] * m[1]  * m[7]  - m[12] * m[3]  * m[5];
        inv[11] = -m[0]  * m[5]  * m[14] + m[0]  * m[6]  * m[13] + m[4]  * m[1]  * m[14] - 
                   m[4]  * m[2]  * m[13] - m[12] * m[1]  * m[6]  + m[12] * m[2]  * m[5];
        
        inv[12] = -m[1]  * m[6]  * m[11] + m[1]  * m[7]  * m[10] + m[5]  * m[2]  * m[11] - 
                   m[5]  * m[3]  * m[10] - m[9]  * m[2]  * m[7]  + m[9]  * m[3]  * m[6];
        inv[13] = m[0]  * m[6]  * m[11] - m[0]  * m[7]  * m[10] - m[4]  * m[2]  * m[11] + 
                  m[4]  * m[3]  * m[10] + m[8]  * m[2]  * m[7]  - m[8]  * m[3]  * m[6];
        inv[14] = -m[0]  * m[5]  * m[11] + m[0]  * m[7]  * m[9]  + m[4]  * m[1]  * m[11] - 
                   m[4]  * m[3]  * m[9]  - m[8]  * m[1]  * m[7]  + m[8]  * m[3]  * m[5];
        inv[15] = m[0]  * m[5]  * m[10] - m[0]  * m[6]  * m[9]  - m[4]  * m[1]  * m[10] + 
                  m[4]  * m[2]  * m[9]  + m[8]  * m[1]  * m[6]  - m[8]  * m[2]  * m[5];
        
        f32 det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        
        if (std::abs(det) < Math::EPSILON) return Identity();
        
        det = 1.0f / det;
        Mat4 result;
        for (i32 i = 0; i < 16; i++) result.m[i] = inv[i] * det;
        return result;
    }
    
    // Factory methods
    static Mat4 Identity() { return Mat4(1.0f); }
    
    static Mat4 Translation(const Vec3& t) {
        Mat4 m(1.0f);
        m.m03 = t.x;
        m.m13 = t.y;
        m.m23 = t.z;
        return m;
    }
    
    static Mat4 Scale(const Vec3& s) {
        Mat4 m(1.0f);
        m.m00 = s.x;
        m.m11 = s.y;
        m.m22 = s.z;
        return m;
    }
    
    static Mat4 Rotation(const Quat& q) {
        f32 xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        f32 xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        f32 wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
        
        Mat4 m(1.0f);
        m.m00 = 1.0f - 2.0f * (yy + zz);
        m.m01 = 2.0f * (xy + wz);
        m.m02 = 2.0f * (xz - wy);
        m.m10 = 2.0f * (xy - wz);
        m.m11 = 1.0f - 2.0f * (xx + zz);
        m.m12 = 2.0f * (yz + wx);
        m.m20 = 2.0f * (xz + wy);
        m.m21 = 2.0f * (yz - wx);
        m.m22 = 1.0f - 2.0f * (xx + yy);
        return m;
    }
    
    static Mat4 TRS(const Vec3& translation, const Quat& rotation, const Vec3& scale) {
        return Translation(translation) * Rotation(rotation) * Scale(scale);
    }
    
    static Mat4 Perspective(f32 fovY, f32 aspect, f32 nearPlane, f32 farPlane) {
        f32 tanHalfFov = std::tan(fovY * 0.5f);
        
        Mat4 m(0.0f);
        m.m00 = 1.0f / (aspect * tanHalfFov);
        m.m11 = 1.0f / tanHalfFov;
        m.m22 = farPlane / (nearPlane - farPlane);
        m.m23 = -1.0f;
        m.m32 = farPlane * nearPlane / (nearPlane - farPlane);
        return m;
    }
    
    static Mat4 Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearPlane, f32 farPlane) {
        Mat4 m(1.0f);
        m.m00 = 2.0f / (right - left);
        m.m11 = 2.0f / (top - bottom);
        m.m22 = 1.0f / (farPlane - nearPlane);
        m.m03 = -(right + left) / (right - left);
        m.m13 = -(top + bottom) / (top - bottom);
        m.m23 = -nearPlane / (farPlane - nearPlane);
        return m;
    }
    
    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up = Vec3::Up()) {
        Vec3 f = (eye - target).Normalized();
        Vec3 r = up.Cross(f).Normalized();
        Vec3 u = f.Cross(r);
        
        Mat4 m(1.0f);
        m.m00 = r.x; m.m01 = r.y; m.m02 = r.z;
        m.m10 = u.x; m.m11 = u.y; m.m12 = u.z;
        m.m20 = f.x; m.m21 = f.y; m.m22 = f.z;
        m.m03 = -r.Dot(eye);
        m.m13 = -u.Dot(eye);
        m.m23 = -f.Dot(eye);
        return m;
    }
};

inline Quat Quat::ToMatrix() const {
    return Quat(x, y, z, w);
}

// ============================================================================
// Transform
// ============================================================================

struct Transform {
    Vec3 position = Vec3::Zero();
    Quat rotation = Quat::Identity();
    Vec3 scale = Vec3::One();
    
    Mat4 GetMatrix() const {
        return Mat4::TRS(position, rotation, scale);
    }
    
    Vec3 Forward() const { return rotation * Vec3::Forward(); }
    Vec3 Right() const { return rotation * Vec3::Right(); }
    Vec3 Up() const { return rotation * Vec3::Up(); }
    
    void LookAt(const Vec3& target, const Vec3& worldUp = Vec3::Up()) {
        Vec3 forward = (target - position).Normalized();
        rotation = Quat::LookRotation(forward, worldUp);
    }
    
    Transform Lerp(const Transform& target, f32 t) const {
        return Transform{
            position.Lerp(target.position, t),
            rotation.Slerp(target.rotation, t),
            scale.Lerp(target.scale, t)
        };
    }
};

// ============================================================================
// Geometric Types
// ============================================================================

struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray() = default;
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.Normalized()) {}
    
    Vec3 At(f32 t) const { return origin + direction * t; }
};

struct Plane {
    Vec3 normal;
    f32 distance;
    
    Plane() = default;
    Plane(const Vec3& n, f32 d) : normal(n.Normalized()), distance(d) {}
    Plane(const Vec3& p1, const Vec3& p2, const Vec3& p3) {
        normal = (p2 - p1).Cross(p3 - p1).Normalized();
        distance = normal.Dot(p1);
    }
    
    f32 DistanceTo(const Vec3& point) const {
        return normal.Dot(point) - distance;
    }
    
    bool Intersect(const Ray& ray, f32& t) const {
        f32 denom = normal.Dot(ray.direction);
        if (std::abs(denom) < Math::EPSILON) return false;
        
        t = distance - normal.Dot(ray.origin);
        t /= denom;
        return t >= 0;
    }
};

struct AABB {
    Vec3 min;
    Vec3 max;
    
    AABB() : min(Vec3(Math::MAX_F32)), max(Vec3(Math::MIN_F32)) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    
    Vec3 Center() const { return (min + max) * 0.5f; }
    Vec3 Extents() const { return (max - min) * 0.5f; }
    Vec3 Size() const { return max - min; }
    f32 Volume() const { Vec3 s = Size(); return s.x * s.y * s.z; }
    
    void Encapsulate(const Vec3& point) {
        min = Vec3(std::min(min.x, point.x), std::min(min.y, point.y), std::min(min.z, point.z));
        max = Vec3(std::max(max.x, point.x), std::max(max.y, point.y), std::max(max.z, point.z));
    }
    
    void Encapsulate(const AABB& other) {
        Encapsulate(other.min);
        Encapsulate(other.max);
    }
    
    bool Contains(const Vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    bool Intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
    
    bool Intersects(const Ray& ray, f32& t) const {
        Vec3 invDir = Vec3(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
        Vec3 t1 = (min - ray.origin) * invDir;
        Vec3 t2 = (max - ray.origin) * invDir;
        
        f32 tmin = std::max(std::max(std::min(t1.x, t2.x), std::min(t1.y, t2.y)), std::min(t1.z, t2.z));
        f32 tmax = std::min(std::min(std::max(t1.x, t2.x), std::max(t1.y, t2.y)), std::max(t1.z, t2.z));
        
        t = tmin;
        return tmax >= 0 && tmax >= tmin;
    }
    
    AABB Transform(const Mat4& matrix) const {
        Vec3 corners[8] = {
            Vec3(min.x, min.y, min.z), Vec3(max.x, min.y, min.z),
            Vec3(min.x, max.y, min.z), Vec3(max.x, max.y, min.z),
            Vec3(min.x, min.y, max.z), Vec3(max.x, min.y, max.z),
            Vec3(min.x, max.y, max.z), Vec3(max.x, max.y, max.z)
        };
        
        AABB result;
        for (const auto& corner : corners) {
            result.Encapsulate(matrix.TransformPoint(corner));
        }
        return result;
    }
    
    static AABB FromCenterSize(const Vec3& center, const Vec3& size) {
        Vec3 half = size * 0.5f;
        return AABB(center - half, center + half);
    }
};

struct Sphere {
    Vec3 center;
    f32 radius;
    
    Sphere() : center(Vec3::Zero()), radius(0) {}
    Sphere(const Vec3& c, f32 r) : center(c), radius(r) {}
    
    bool Contains(const Vec3& point) const {
        return (point - center).LengthSq() <= radius * radius;
    }
    
    bool Intersects(const Sphere& other) const {
        f32 distSq = (center - other.center).LengthSq();
        f32 radiusSum = radius + other.radius;
        return distSq <= radiusSum * radiusSum;
    }
    
    bool Intersects(const AABB& aabb) const {
        Vec3 closest = Vec3(
            std::clamp(center.x, aabb.min.x, aabb.max.x),
            std::clamp(center.y, aabb.min.y, aabb.max.y),
            std::clamp(center.z, aabb.min.z, aabb.max.z)
        );
        return (center - closest).LengthSq() <= radius * radius;
    }
};

struct Frustum {
    Plane planes[6]; // left, right, bottom, top, near, far
    
    static Frustum FromMatrix(const Mat4& viewProj) {
        Frustum f;
        Mat4 m = viewProj;
        
        // Left
        f.planes[0] = Plane(
            Vec3(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]),
            m[3][3] + m[3][0]
        );
        // Right
        f.planes[1] = Plane(
            Vec3(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]),
            m[3][3] - m[3][0]
        );
        // Bottom
        f.planes[2] = Plane(
            Vec3(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]),
            m[3][3] + m[3][1]
        );
        // Top
        f.planes[3] = Plane(
            Vec3(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]),
            m[3][3] - m[3][1]
        );
        // Near
        f.planes[4] = Plane(
            Vec3(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]),
            m[3][3] + m[3][2]
        );
        // Far
        f.planes[5] = Plane(
            Vec3(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]),
            m[3][3] - m[3][2]
        );
        
        for (auto& plane : f.planes) {
            f32 len = plane.normal.Length();
            plane.normal /= len;
            plane.distance /= len;
        }
        
        return f;
    }
    
    bool Contains(const Vec3& point) const {
        for (const auto& plane : planes) {
            if (plane.DistanceTo(point) < 0) return false;
        }
        return true;
    }
    
    bool Intersects(const AABB& aabb) const {
        for (const auto& plane : planes) {
            Vec3 positive = aabb.min;
            if (plane.normal.x > 0) positive.x = aabb.max.x;
            if (plane.normal.y > 0) positive.y = aabb.max.y;
            if (plane.normal.z > 0) positive.z = aabb.max.z;
            
            if (plane.DistanceTo(positive) < 0) return false;
        }
        return true;
    }
    
    bool Intersects(const Sphere& sphere) const {
        for (const auto& plane : planes) {
            if (plane.DistanceTo(sphere.center) < -sphere.radius) return false;
        }
        return true;
    }
};

// ============================================================================
// Random Number Generation
// ============================================================================

class Random {
public:
    Random() : m_rng(std::random_device{}()) {}
    explicit Random(u64 seed) : m_rng(seed) {}
    
    f32 Float() {
        return std::uniform_real_distribution<f32>(0.0f, 1.0f)(m_rng);
    }
    
    f32 Range(f32 min, f32 max) {
        return std::uniform_real_distribution<f32>(min, max)(m_rng);
    }
    
    i32 RangeInt(i32 min, i32 max) {
        return std::uniform_int_distribution<i32>(min, max)(m_rng);
    }
    
    Vec2 InCircle(f32 radius = 1.0f) {
        f32 r = radius * std::sqrt(Float());
        f32 theta = Range(0.0f, Math::TWO_PI);
        return Vec2(r * std::cos(theta), r * std::sin(theta));
    }
    
    Vec3 InSphere(f32 radius = 1.0f) {
        f32 u = Float();
        f32 v = Float();
        f32 theta = Range(0.0f, Math::TWO_PI);
        f32 phi = std::acos(2.0f * v - 1.0f);
        f32 r = radius * std::cbrt(u);
        
        return Vec3(
            r * std::sin(phi) * std::cos(theta),
            r * std::sin(phi) * std::sin(theta),
            r * std::cos(phi)
        );
    }
    
    Vec3 Hemisphere(const Vec3& normal) {
        Vec3 dir = InSphere(1.0f);
        if (dir.Dot(normal) < 0) dir = -dir;
        return dir;
    }
    
private:
    std::mt19937_64 m_rng;
};

// ============================================================================
// Utility Functions
// ============================================================================

inline f32 Radians(f32 degrees) {
    return degrees * Math::DEG_TO_RAD;
}

inline f32 Degrees(f32 radians) {
    return radians * Math::RAD_TO_DEG;
}

inline f32 Clamp(f32 value, f32 min, f32 max) {
    return std::clamp(value, min, max);
}

inline f32 Clamp01(f32 value) {
    return std::clamp(value, 0.0f, 1.0f);
}

inline f32 Lerp(f32 a, f32 b, f32 t) {
    return a + (b - a) * Clamp01(t);
}

inline f32 InverseLerp(f32 a, f32 b, f32 value) {
    return (value - a) / (b - a);
}

inline f32 Remap(f32 value, f32 inMin, f32 inMax, f32 outMin, f32 outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

inline f32 MoveTowards(f32 current, f32 target, f32 maxDelta) {
    if (std::abs(target - current) <= maxDelta) return target;
    return current + std::copysign(maxDelta, target - current);
}

inline f32 PingPong(f32 value, f32 length) {
    value = std::fmod(value, length * 2.0f);
    if (value < 0) value += length * 2.0f;
    return length - std::abs(value - length);
}

inline f32 WrapAngle(f32 angle) {
    angle = std::fmod(angle, Math::TWO_PI);
    if (angle > Math::PI) angle -= Math::TWO_PI;
    if (angle < -Math::PI) angle += Math::TWO_PI;
    return angle;
}

inline f32 AngleBetween(const Vec3& a, const Vec3& b) {
    f32 dot = a.Dot(b);
    f32 len = a.Length() * b.Length();
    return std::acos(Clamp(dot / len, -1.0f, 1.0f));
}

inline f32 SignedAngle(const Vec3& from, const Vec3& to, const Vec3& axis) {
    f32 angle = AngleBetween(from, to);
    f32 sign = std::copysign(1.0f, axis.Dot(from.Cross(to)));
    return angle * sign;
}

inline Vec3 ProjectOnPlane(const Vec3& v, const Vec3& normal) {
    return v - normal * v.Dot(normal);
}

inline f32 SmoothStep(f32 t) {
    t = Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

inline f32 SmootherStep(f32 t) {
    t = Clamp01(t);
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// Easing functions
namespace Easing {
    inline f32 Linear(f32 t) { return t; }
    inline f32 InQuad(f32 t) { return t * t; }
    inline f32 OutQuad(f32 t) { return 1.0f - (1.0f - t) * (1.0f - t); }
    inline f32 InOutQuad(f32 t) { return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f; }
    inline f32 InCubic(f32 t) { return t * t * t; }
    inline f32 OutCubic(f32 t) { return 1.0f - std::pow(1.0f - t, 3.0f); }
    inline f32 InOutCubic(f32 t) { return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f; }
    inline f32 InQuart(f32 t) { return t * t * t * t; }
    inline f32 OutQuart(f32 t) { return 1.0f - std::pow(1.0f - t, 4.0f); }
    inline f32 InOutQuart(f32 t) { return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) * 0.5f; }
    inline f32 InQuint(f32 t) { return t * t * t * t * t; }
    inline f32 OutQuint(f32 t) { return 1.0f - std::pow(1.0f - t, 5.0f); }
    inline f32 InOutQuint(f32 t) { return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) * 0.5f; }
    inline f32 InSine(f32 t) { return 1.0f - std::cos(t * Math::HALF_PI); }
    inline f32 OutSine(f32 t) { return std::sin(t * Math::HALF_PI); }
    inline f32 InOutSine(f32 t) { return -(std::cos(Math::PI * t) - 1.0f) * 0.5f; }
    inline f32 InExpo(f32 t) { return t == 0 ? 0 : std::pow(2.0f, 10.0f * t - 10.0f); }
    inline f32 OutExpo(f32 t) { return t == 1 ? 1 : 1.0f - std::pow(2.0f, -10.0f * t); }
    inline f32 InOutExpo(f32 t) { 
        return t == 0 ? 0 : t == 1 ? 1 : t < 0.5f ? std::pow(2.0f, 20.0f * t - 10.0f) * 0.5f : (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) * 0.5f;
    }
    inline f32 InCirc(f32 t) { return 1.0f - std::sqrt(1.0f - std::pow(t, 2.0f)); }
    inline f32 OutCirc(f32 t) { return std::sqrt(1.0f - std::pow(t - 1.0f, 2.0f)); }
    inline f32 InOutCirc(f32 t) { return t < 0.5f ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * t, 2.0f))) * 0.5f : (std::sqrt(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) * 0.5f; }
    inline f32 InBack(f32 t) { f32 c1 = 1.70158f; f32 c3 = c1 + 1.0f; return c3 * t * t * t - c1 * t * t; }
    inline f32 OutBack(f32 t) { f32 c1 = 1.70158f; f32 c3 = c1 + 1.0f; return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f); }
    inline f32 InOutBack(f32 t) { f32 c1 = 1.70158f; f32 c2 = c1 * 1.525f; return t < 0.5f ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) * 0.5f : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) * 0.5f; }
    inline f32 InElastic(f32 t) { f32 c4 = (2.0f * Math::PI) / 3.0f; return t == 0 ? 0 : t == 1 ? 1 : -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4); }
    inline f32 OutElastic(f32 t) { f32 c4 = (2.0f * Math::PI) / 3.0f; return t == 0 ? 0 : t == 1 ? 1 : std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f; }
    inline f32 InOutElastic(f32 t) { f32 c5 = (2.0f * Math::PI) / 4.5f; return t == 0 ? 0 : t == 1 ? 1 : t < 0.5f ? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) * 0.5f : (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) * 0.5f + 1.0f; }
    inline f32 OutBounce(f32 t) { f32 n1 = 7.5625f; f32 d1 = 2.75f; if (t < 1.0f / d1) { return n1 * t * t; } else if (t < 2.0f / d1) { return n1 * (t -= 1.5f / d1) * t + 0.75f; } else if (t < 2.5f / d1) { return n1 * (t -= 2.25f / d1) * t + 0.9375f; } else { return n1 * (t -= 2.625f / d1) * t + 0.984375f; } }
    inline f32 InBounce(f32 t) { return 1.0f - OutBounce(1.0f - t); }
    inline f32 InOutBounce(f32 t) { return t < 0.5f ? (1.0f - OutBounce(1.0f - 2.0f * t)) * 0.5f : (1.0f + OutBounce(2.0f * t - 1.0f)) * 0.5f; }
}

} // namespace Duality