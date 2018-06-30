#pragma once

#include "AccelerationStructureBase.h"
#include "Core/Geometry/GeometryBase.h"
#include <vector>

namespace Petrichor
{
namespace Core
{

struct HitInfo;
struct Ray;
class Scene;
class GeometryBase;

class BruteForce : public AccelerationStructureBase
{
public:
    void
    Build(const Scene& scene) override;

    std::optional<HitInfo>
    Intersect(const Ray& ray,
              float distMin = 0.0f,
              float distMax = kInfinity) const override;

private:
    std::vector<const GeometryBase*> m_geometries;
};

} // namespace Core
} // namespace Petrichor
