#include "BVHNode.h"

#include <Core/Constants.h>
#include <Core/Geometry/GeometryBase.h>
#include <Core/HitInfo.h>
#include <Core/Ray.h>

namespace Petrichor
{
namespace Core
{

using namespace Math;

std::optional<HitInfo>
BVHNode::Intersect(const Ray& ray) const
{
    Vector3f invRayDir = Vector3f::One() / ray.dir;
    const Vector3f t0 = (GetBounds().vMin - ray.o) * invRayDir;
    const Vector3f t1 = (GetBounds().vMax - ray.o) * invRayDir;

    constexpr uint8_t idxArray[] = { 0, 1, 2, 0 };

    for (uint8_t cnt = 0; cnt < 3; cnt++)
    {
        const uint8_t axis0 = idxArray[cnt];
        const uint8_t axis1 = idxArray[cnt + 1];

        if (ray.dir[axis0] == 0 || ray.dir[axis1] == 0)
        {
            // 軸に平行なレイが入ってきた場合
            continue;
        }

        const float tMinMax = std::max(std::min(t0[axis0], t1[axis0]),
                                       std::min(t0[axis1], t1[axis1]));
        const float tMaxMin = std::min(std::max(t0[axis0], t1[axis0]),
                                       std::max(t0[axis1], t1[axis1]));
        if (tMaxMin < tMinMax)
        {
            return std::nullopt;
        }
    }

    float distance = kInfinity;
    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        if (t0[axis] > 0.0f)
        {
            distance = std::min(distance, t0[axis]);
        }

        if (t1[axis] > 0.0f)
        {
            distance = std::min(distance, t1[axis]);
        }
    }

    HitInfo hitInfo;
    hitInfo.distance = distance;
    // BVHNodeは実際に接触する物体ではないので、その他hitInfoの更新は行わない
    return hitInfo;
}

bool
BVHNode::Contains(const Math::Vector3f& point) const
{
    const Math::Vector3f diff = GetBounds().vMax - GetBounds().vMin;
    const Math::Vector3f point2 = point - GetBounds().vMin;

    if (diff.x < point2.x || diff.y < point2.y || diff.z < point2.z)
    {
        return false;
    }

    return true;
}

} // namespace Core
} // namespace Petrichor
