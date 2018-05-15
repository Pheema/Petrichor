#pragma once

#include "Core/Constants.h"
#include <limits>

namespace Petrichor
{
namespace Core
{

struct HitInfo;
struct Ray;
class Scene;

enum class AccelType
{
    BruteForce, // 構造を使用しない
    BVH         // BVH
};

class AccelerationStructureBase
{
public:
    virtual void
    Build(const Scene& scene) = 0;

    virtual bool
    Intersect(const Ray& ray,
              HitInfo* hitInfo,
              float distMin = 0.0f,
              float distMax = kInfinity) const = 0;
};

} // namespace Core
} // namespace Petrichor
