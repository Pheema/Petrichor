#pragma once

#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

class Bound
{
public:
    Bound();

    Bound(const Math::Vector3f& vMin, const Math::Vector3f& vMax);

    // Boundの中心を求める
    Math::Vector3f
    Center() const;

    // 複数のBoundをマージする
    void
    Expand(const Bound& aabb);

    void
    Expand(const Math::Vector3f& point);

    // 一番広い辺を取得する
    uint8_t GetWidestAxis() const;

    Math::Vector3f vMin, vMax;
};

#pragma region Inline functions

inline Math::Vector3f
Bound::Center() const
{
    return 0.5f * (vMin + vMax);
}

#pragma endregion

} // namespace Core
} // namespace Petrichor
