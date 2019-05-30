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

std::optional<HitInfo>
BruteForce::Intersect(const Ray& ray, float distMin, float distMax) const
{
    std::optional<HitInfo> hitInfoResult;

    for (const auto geometry : m_geometries)
    {
        if (const auto geoHitInfo = geometry->Intersect(ray); geoHitInfo)
        {
            // 衝突位置がレイの原点から近すぎたり遠すぎる場合は無視
            if (geoHitInfo->distance < distMin ||
                geoHitInfo->distance > distMax)
            {
                continue;
            }

            // 衝突位置が近ければ衝突情報を更新
            if (hitInfoResult == std::nullopt ||
                geoHitInfo->distance < hitInfoResult->distance)
            {
                hitInfoResult = geoHitInfo;
            }
        }
    }

    return hitInfoResult;
}

} // namespace Core
} // namespace Petrichor
