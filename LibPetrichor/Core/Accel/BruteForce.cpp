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

std::optional<Petrichor::Core::HitInfo>
BruteForce::Intersect(const Ray& ray,
                      float distMin /*= 0.0f*/,
                      float distMax /*= kInfinity*/) const
{
    std::optional<HitInfo> hitInfoResult;

    for (const auto* geometry : m_geometries)
    {
        if (const auto optGeoHitInfo = geometry->Intersect(ray); optGeoHitInfo)
        {
            const auto& geoHitInfo = optGeoHitInfo.value();

            // 衝突位置がレイの原点から近すぎたり遠すぎる場合は無視
            if (geoHitInfo.distance < distMin || geoHitInfo.distance > distMax)
            {
                continue;
            }

            // 衝突位置が近ければ衝突情報を更新
            if (hitInfoResult == std::nullopt ||
                geoHitInfo.distance < hitInfoResult.value().distance)
            {
                hitInfoResult = geoHitInfo;
            }
        }
    }

    return hitInfoResult;
}

} // namespace Core
} // namespace Petrichor
