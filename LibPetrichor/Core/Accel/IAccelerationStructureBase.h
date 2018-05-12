#pragma once

#include <limits>
#include <Core/Constants.h>

namespace Petrichor
{
namespace Core
{

struct HitInfo;
struct Ray;
class Scene;

class IAccelerationStructureBase
{
public:
    virtual void Build(const Scene& scene) = 0;
    
    virtual bool Intersect(
        const Ray& ray,
        HitInfo* hitInfo,
        float distMin = 0.0f,
        float distMax = kInfinity
    ) const = 0;
};

}   // namespace Core
}   // namespace Petrichor
