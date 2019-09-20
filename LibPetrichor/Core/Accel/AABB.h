#pragma once

#include "Math/Vector3f.h"
#include <cstdint>

namespace Petrichor
{
namespace Core
{

struct AABB
{
    AABB() = default;

    AABB(const Math::Vector3f& lower, const Math::Vector3f& upper)
      : lower(lower)
      , upper(upper)
    {
    }

    inline bool
    Contains(const Math::Vector3f& point) const;

    // Boundの中心を求める
    inline Math::Vector3f
    CalcCentroid() const;

    // 表面積を求める
    inline float
    GetSurfaceArea() const;

    // 複数のBoundをマージする
    void
    Merge(const AABB& bound);

    void
    Merge(const Math::Vector3f& point);

    //! どの軸に対して一番辺が広いかを取得する
    //! @return 軸番号(X: 0, Y: 1, Z: 2)
    int
    GetWidestAxis() const;

    const Math::Vector3f& operator[](int i) const
    {
        ASSERT(0 <= i && i < 2);
        return i ? upper : lower;
    }

    Math::Vector3f lower =
      Math::Vector3f::One() * std::numeric_limits<float>::max();
    Math::Vector3f upper =
      Math::Vector3f::One() * std::numeric_limits<float>::lowest();
};

#pragma region Inline functions

bool
AABB::Contains(const Math::Vector3f& point) const
{
    if (lower.x < point.x && point.x < upper.x && lower.y < point.y &&
        point.y < upper.y && lower.z < point.z && point.z < upper.z)
    {
        return true;
    }

    return false;
}

Math::Vector3f
AABB::CalcCentroid() const
{
    return 0.5f * (lower + upper);
}

float
AABB::GetSurfaceArea() const
{
    const Math::Vector3f diff = upper - lower;
    return 2.0f * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
}

#pragma endregion

} // namespace Core
} // namespace Petrichor
