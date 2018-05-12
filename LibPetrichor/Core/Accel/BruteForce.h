#pragma once


#include <vector>
#include "IAccelerationStructureBase.h"
#include <Core/Geometry/GeometryBase.h>

namespace Petrichor
{
namespace Core
{

struct HitInfo;
struct Ray;
class Scene;
class GeometryBase;

class BruteForce : public IAccelerationStructureBase
{
public:
    void
    Build(const Scene& scene) override;
    
    bool
    Intersect(
        const Ray& ray,
        HitInfo* hitInfo,
        float distMin = 0.0f,
        float distMax = std::numeric_limits<float>::max()
    ) const override;

private:
    std::vector<const GeometryBase*> m_geometries;

};

}   // namespace Core
}   // namespace Petrichor
