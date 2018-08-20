#pragma once

#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

class Bounds
{
public:
    Bounds();

    Bounds(const Math::Vector3f& vMin, const Math::Vector3f& vMax);

    // Boundの中心を求める
    Math::Vector3f
    GetCenter() const;

    // 表面積を求める
    float
    GetSurfaceArea() const;

    // 複数のBoundをマージする
    void
    Merge(const Bounds& bound);

    void
    Merge(const Math::Vector3f& point);

    //! どの軸に対して一番辺が広いかを取得する
    //! @return 軸番号(X: 0, Y: 1, Z: 2)
    int
    GetWidestAxis() const;

    Math::Vector3f vMin, vMax;
};

#pragma region Inline functions

inline Math::Vector3f
Bounds::GetCenter() const
{
    return 0.5f * (vMin + vMax);
}

#pragma endregion

} // namespace Core
} // namespace Petrichor
