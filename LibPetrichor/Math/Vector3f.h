#pragma once

#include "Core/Assert.h"
#include "Math/MathUtils.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace Petrichor
{
namespace Math
{

class Vector3f
{
public:
    constexpr Vector3f() = default;

    constexpr Vector3f(float x, float y, float z)
      : x(x)
      , y(y)
      , z(z){};

    // Get Constants
    constexpr static Vector3f
    Zero();

    constexpr static Vector3f
    One();

    constexpr static Vector3f
    UnitX();

    constexpr static Vector3f
    UnitY();

    constexpr static Vector3f
    UnitZ();

    // Length
    float
    Length() const;
    constexpr float
    SquaredLength() const;

    // Normalization
    void
    Normalize();
    Vector3f
    Normalized() const;

    // Min, Max
    float
    MaxElem() const;
    float
    MinElem() const;

    // normalは正規化されている前提
    Vector3f
    Reflected(const Vector3f& normal) const;

    // normalは正規化されている前提
    // 全反射になってしまう場合は返り値として(0, 0, 0)が渡される
    Vector3f
    Refracted(const Vector3f& normal, float relativeIOR) const;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

#pragma region Operator overloading

    // Addition
    Vector3f
    operator+(const Vector3f& v) const;
    const Vector3f&
    operator+=(const Vector3f& v);
    Vector3f
    operator+() const;

    // Subtraction
    Vector3f
    operator-(const Vector3f& v) const;
    Vector3f&
    operator-=(const Vector3f& v);
    Vector3f
    operator-() const;

    // Multiplication
    Vector3f operator*(const Vector3f& v) const;
    Vector3f operator*(float c) const;
    const Vector3f&
    operator*=(const Vector3f& v);
    const Vector3f&
    operator*=(float c);
    friend Vector3f operator*(float c, const Vector3f& v);

    // Division
    Vector3f
    operator/(const Vector3f& v) const;

    Vector3f
    operator/(float c) const;

    const Vector3f&
    operator/=(const Vector3f& v);

    const Vector3f&
    operator/=(float c);

    friend Vector3f
    operator/(float c, const Vector3f& v);

    constexpr float operator[](int i) const;

    constexpr float& Vector3f::operator[](int i);

    // iostream
    friend std::ostream&
    operator<<(std::ostream& os, const Vector3f& v);

#pragma endregion
};

#pragma region Inline functions

constexpr float
Dot(const Vector3f& v0, const Vector3f& v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

constexpr Vector3f
Cross(const Vector3f& v0, const Vector3f& v1)
{
    return Vector3f(v0.y * v1.z - v0.z * v1.y,
                    v0.z * v1.x - v0.x * v1.z,
                    v0.x * v1.y - v0.y * v1.x);
}

constexpr Vector3f
Vector3f::Zero()
{
    return Vector3f(0.0f, 0.0f, 0.0f);
}
constexpr Vector3f
Vector3f::One()
{
    return Vector3f(1.0f, 1.0f, 1.0f);
}

constexpr Vector3f
Vector3f::UnitY()
{
    return Vector3f(0.0f, 1.0f, 0.0f);
}
constexpr Vector3f
Vector3f::UnitX()
{
    return Vector3f(1.0f, 0.0f, 0.0f);
}
constexpr Vector3f
Vector3f::UnitZ()
{
    return Vector3f(0.0f, 0.0f, 1.0f);
}

inline float
Vector3f::Length() const
{
    return sqrt(Dot(*this, *this));
}
constexpr float
Vector3f::SquaredLength() const
{
    return Dot(*this, *this);
}

inline void
Vector3f::Normalize()
{
    *this /= Length();
}
[[nodiscard]] inline Vector3f
Vector3f::Normalized() const
{
    return *this / Length();
}

inline float
Vector3f::MaxElem() const
{
    return std::max(x, std::max(y, z));
}
inline float
Vector3f::MinElem() const
{
    return std::min(x, std::min(y, z));
}

inline Vector3f
Vector3f::Reflected(const Vector3f& normal) const
{
    IS_NORMALIZED(normal);
    return *this - 2.0f * normal * Dot(*this, normal);
}

inline Vector3f
Vector3f::operator+(const Vector3f& v) const
{
    return Vector3f(x + v.x, y + v.y, z + v.z);
}

inline const Vector3f&
Vector3f::operator+=(const Vector3f& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

inline Vector3f
Vector3f::operator+() const
{
    return *this;
}

inline Vector3f
Vector3f::operator-(const Vector3f& v) const
{
    return Vector3f(x - v.x, y - v.y, z - v.z);
}

inline Vector3f&
Vector3f::operator-=(const Vector3f& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

inline Vector3f
Vector3f::operator-() const
{
    return Vector3f(-x, -y, -z);
}

inline Vector3f Vector3f::operator*(const Vector3f& v) const
{
    return Vector3f(x * v.x, y * v.y, z * v.z);
}

inline Vector3f Vector3f::operator*(float c) const
{
    return Vector3f(x * c, y * c, z * c);
}

inline const Vector3f&
Vector3f::operator*=(const Vector3f& v)
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

inline const Vector3f&
Vector3f::operator*=(float c)
{
    x *= c;
    y *= c;
    z *= c;
    return *this;
}

inline Vector3f
Vector3f::operator/(const Vector3f& v) const
{
    return Vector3f(x / v.x, y / v.y, z / v.z);
}

inline Vector3f
Vector3f::operator/(float c) const
{
    return Vector3f(x / c, y / c, z / c);
}

inline const Vector3f&
Vector3f::operator/=(const Vector3f& v)
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

inline const Vector3f&
Vector3f::operator/=(float c)
{
    x /= c;
    y /= c;
    z /= c;
    return *this;
}

constexpr float Vector3f::operator[](int i) const
{
    ASSERT(0 <= i && i < 3);
    return *(&x + i);
}

constexpr float& Vector3f::operator[](int i)
{
    ASSERT(0 <= i && i < 3);
    return *(&x + i);
}

inline Vector3f operator*(float c, const Vector3f& v)
{
    return v * c;
}

inline Vector3f
operator/(float c, const Vector3f& v)
{
    return Vector3f(c / v.x, c / v.y, c / v.z);
}

inline std::ostream&
operator<<(std::ostream& os, const Vector3f& v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

//!
inline bool
ApproxEq(const Vector3f& v0, const Vector3f& v1, float squaredEps)
{
    float sqLength = (v1 - v0).SquaredLength();
    return Math::ApproxEq(sqLength, 0.0f, squaredEps);
}

#pragma endregion

} // namespace Math
} // namespace Petrichor
