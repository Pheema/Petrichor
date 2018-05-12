#include "BruteForce.h"

#include "Core/Geometry/GeometryBase.h"
#include "Core/HitInfo.h"
#include "Core/Scene.h"

namespace Petrichor
{
namespace Core
{

void
BruteForce::Build(const Scene& scene)
{
    for (const auto* geometry : scene.GetGeometries())
    {
        m_geometries.emplace_back(geometry);
    }
}

bool
BruteForce::Intersect(const Ray& ray,
                      HitInfo* hitInfo,
                      float distMin,
                      float distMax) const
{
    bool isHit = false;
    for (const auto* geometry : m_geometries)
    {
        HitInfo hitInfo_;
        if (geometry->Intersect(ray, &hitInfo_))
        {
            // 衝突位置がレイの原点から近すぎたり遠すぎる場合は無視
            if (hitInfo_.distance < distMin || hitInfo_.distance > distMax)
            {
                continue;
            }

            // 衝突位置が近ければ衝突情報を更新
            if (hitInfo_.distance < hitInfo->distance)
            {
                isHit    = true;
                *hitInfo = hitInfo_;
            }
        }
    }

    return isHit;
}

} // namespace Core
} // namespace Petrichor
