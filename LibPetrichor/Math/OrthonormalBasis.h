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
    constexpr OrthonormalBasis() = default;

    // 法線ベクトルを指定した正規直交基底の構築
    void
    Build(const Vector3f& w);

    // 法線ベクトル、接線ベクトルを指定した正規直交基底の構築
    void
    Build(const Vector3f& w, const Vector3f& u);

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
    GetU() const;

    inline const Vector3f&
    GetV() const;

    inline const Vector3f&
    GetW() const;

private:
    Vector3f m_u; //!< 接線方向(LocalX)
    Vector3f m_v; //!< 従法線方向(LocalY)
    Vector3f m_w; //!< 法線方向(LocalZ)
};

#pragma region Inline functions

inline const Vector3f&
OrthonormalBasis::GetU() const
{
    return m_u;
}

inline const Vector3f&
OrthonormalBasis::GetV() const
{
    return m_v;
}

inline const Vector3f&
OrthonormalBasis::GetW() const
{
    return m_w;
}

#pragma endregion
} // namespace Math
} // namespace Petrichor
