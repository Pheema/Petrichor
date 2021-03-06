#pragma once

#include "AccelBase.h"
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

class BruteForce : public AccelBase
{
public:
    void
    Build(const Scene& scene) override;

    std::optional<HitInfo>
    Intersect(const Ray& ray,
              const Scene& scene,
              float distMin,
              float distMax) const override;

private:
    std::vector<const GeometryBase*> m_geometries;
};

} // namespace Core
} // namespace Petrichor
