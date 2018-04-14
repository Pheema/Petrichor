#pragma once


#include <Math/Vector3f.h>
// #include <Core/Geometry/GeometryBase.h>

namespace Petrichor
{
namespace Core
{
class GeometryBase;

    struct HitInfo
    {
        HitInfo() : 
            pos(),
            normal(),
            uv(),
            distance(kInfinity)
        {
        };

        void Clear()
        {
            pos = Math::Vector3f::Zero();
            normal = Math::Vector3f::Zero();
            uv = Math::Vector3f::Zero();
            distance = kInfinity;
        }

        Math::Vector3f  pos;    // 衝突位置のワールド座標
        Math::Vector3f  normal; // 衝突箇所の単位法線ベクトル
        Math::Vector3f  uv;     // 衝突箇所のUV値
        float           distance; // 反射点から衝突点までの距離
        const GeometryBase* hitObj; // 衝突したジオメトリへのポインタ
    };
}   // namespace Core
}   // namespace Petrichor
