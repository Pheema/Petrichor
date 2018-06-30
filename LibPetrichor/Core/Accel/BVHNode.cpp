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

BVHNode::BVHNode()
  : bound()
{
}

BVHNode::BVHNode(const Vector3f& vMin, const Vector3f& vMax)
  : bound(vMin, vMax){};

std::optional<HitInfo>
BVHNode::Intersect(const Ray& ray) const
{
    // TODO: ray.dirが軸に平行だと0div
    const Vector3f InvRayDir = Vector3f::One() / ray.dir;

    const Vector3f t0 = (bound.vMin - ray.o) * InvRayDir;
    const Vector3f t1 = (bound.vMax - ray.o) * InvRayDir;

    const uint8_t idxArray[] = { 0, 1, 2, 0 };

    for (uint8_t cnt = 0; cnt < 3; cnt++)
    {
        const uint8_t axis0 = idxArray[cnt];
        const uint8_t axis1 = idxArray[cnt + 1];

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
    hitInfo.pos      = ray.o + ray.dir * distance;
    // BVHNodeは実際に接触する物体ではないので、その他hitInfoの更新は行わない
    return hitInfo;
}

size_t
BVHNode::Partition(int axis)
{
    const float center = bound.Center()[axis];

    const auto iterMid =
      std::partition(m_childGeometries.begin(),
                     m_childGeometries.end(),
                     [center, axis](const GeometryBase* obj) {
                         return (obj->CalcBound().Center()[axis] < center);
                     });

    return std::distance(m_childGeometries.begin(), iterMid);
}

void
BVHNode::SortByNthElement(int widestAxis, float center, size_t nth)
{
    std::nth_element(
      m_childGeometries.begin(),
      m_childGeometries.begin() + nth,
      m_childGeometries.end(),
      [center, widestAxis](const GeometryBase* g0, const GeometryBase* g1) {
          return g0->CalcBound().Center()[widestAxis] <
                 g1->CalcBound().Center()[widestAxis];
      });
}

} // namespace Core
} // namespace Petrichor
