#pragma once

#include "IAccelerationStructureBase.h"
#include "BVHNode.h"
#include <Core/Geometry/GeometryBase.h>
#include <Core/HitInfo.h>


namespace Petrichor
{
namespace Core
{

class Scene;
struct HitInfo;
struct Ray;

class BVH : public IAccelerationStructureBase
{
public:
    BVH() = default;

    virtual void
    Build(const Scene& scene) override;

    virtual bool
    Intersect(
        const Ray& ray,
        HitInfo* hitInfo,
        float distMin = 0.0f,
        float distMax = kInfinity
    ) const override;

private:
    std::vector<BVHNode> m_bvhNodes;
};

}   // namespace Core
}   // namespace Petrichor
