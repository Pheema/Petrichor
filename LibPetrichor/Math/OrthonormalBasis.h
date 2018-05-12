#pragma once

#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Math
{

// Ref: [Building an Orthonormal Basis, Revisited]
// https://graphics.pixar.com/library/OrthonormalB/paper.pdf
class OrthonormalBasis
{
public:
    OrthonormalBasis();

    // 法線ベクトルを指定した正規直交基底の構築
    void
    Build(const Vector3f& localZ);

    // 法線ベクトル、接線ベクトルを指定した正規直交基底の構築
    void
    Build(const Vector3f& normal, const Vector3f& tangent);

    // 球面座標の(θ, φ)から単位方向ベクトルを取得する
    Vector3f
    GetDir(float theta, float phi);

    // ワールド座標系で表されるベクトルvecWorldをローカル座標系で表したベクトルを返す
    Vector3f
    WorldToLocal(const Vector3f& vecWorld) const;

    // ローカル座標系で表されるベクトルvecLocalをワールド座標系へ変換する
    Vector3f
    LocalToWorld(const Vector3f& vecLocal) const;

    inline const Vector3f&
    GetBaseX() const;

    inline const Vector3f&
    GetBaseY() const;

    inline const Vector3f&
    GetBaseZ() const;

private:
    Vector3f m_baseX; // 接線方向
    Vector3f m_baseY; // 従法線方向
    Vector3f m_baseZ; // 法線方向
};

#pragma region Inline functions

const Vector3f&
OrthonormalBasis::GetBaseX() const
{
    return m_baseX;
}

const Vector3f&
OrthonormalBasis::GetBaseY() const
{
    return m_baseY;
}

const Vector3f&
OrthonormalBasis::GetBaseZ() const
{
    return m_baseZ;
}

#pragma endregion
} // namespace Math
} // namespace Petrichor
