#pragma once

#include "Bounds.h"
#include "Core/Accel/AccelBase.h"
#include <Core/Geometry/GeometryBase.h>
#include <Core/HitInfo.h>
#include <Core/Ray.h>
#include <array>
#include <optional>
#include <vector>

namespace Petrichor
{
namespace Core
{

class GeometryBase;
struct HitInfo;
struct Ray;

class BVHNode
{
public:
    BVHNode(const Bounds& bounds,
            const std::array<int, 2>& childIndicies,
            int indexBegin,
            int indexEnd,
            bool isLeaf)
      : m_bounds(bounds)
      , m_childNodeIndicies(childIndicies)
      , m_indexBegin(indexBegin)
      , m_indexEnd(indexEnd)
      , m_isLeaf(isLeaf)
    {
    }

    std::optional<HitInfo>
    Intersect(const Ray& ray, const PrecalcedData& precalced) const
    {
        float tMin =
          (GetBounds()[precalced.sign[0]].x - ray.o.x) * precalced.invRayDir.x;
        float tMax = (GetBounds()[1 - precalced.sign[0]].x - ray.o.x) *
                     precalced.invRayDir.x;

        {
            const float tyMin = (GetBounds()[precalced.sign[1]].y - ray.o.y) *
                                precalced.invRayDir.y;
            const float tyMax =
              (GetBounds()[1 - precalced.sign[1]].y - ray.o.y) *
              precalced.invRayDir.y;

            if (tyMax < tMin || tMax < tyMin)
            {
                return std::nullopt;
            }

            if (tMin < tyMin)
            {
                tMin = tyMin;
            }

            if (tyMax < tMax)
            {
                tMax = tyMax;
            }
        }

        {
            const float tzMin = (GetBounds()[precalced.sign[2]].z - ray.o.z) *
                                precalced.invRayDir.z;
            const float tzMax =
              (GetBounds()[1 - precalced.sign[2]].z - ray.o.z) *
              precalced.invRayDir.z;

            if (tzMax < tMin || tMax < tzMin)
            {
                return std::nullopt;
            }

            if (tMin < tzMin)
            {
                tMin = tzMin;
            }

            if (tzMax < tMax)
            {
                tMax = tzMax;
            }
        }

        float distance = std::numeric_limits<float>::max();

        const bool isNearCollisionValid = (0.0f <= tMin && tMin < distance);
        if (isNearCollisionValid)
        {
            distance = tMin;
        }

        const bool isFarCollisionValid = (0.0f <= tMax && tMax < distance);
        if (isFarCollisionValid)
        {
            distance = tMax;
        }

        if (!(isNearCollisionValid || isFarCollisionValid))
        {
            return std::nullopt;
        }

        const HitInfo hitInfo = [&] {
            HitInfo h;
            h.distance = distance;
            return h;
        }();
        return hitInfo;
    }

    bool
    Contains(const Math::Vector3f& point) const
    {
        // #TODO: 判定怪しげ
        if (GetBounds().vMin[0] >= point.x)
        {
            return false;
        }

        if (GetBounds().vMin[1] >= point.y)
        {
            return false;
        }

        if (GetBounds().vMin[2] >= point.z)
        {
            return false;
        }

        if (GetBounds().vMax[0] <= point.x)
        {
            return false;
        }

        if (GetBounds().vMax[1] <= point.y)
        {
            return false;
        }

        if (GetBounds().vMax[2] <= point.z)
        {
            return false;
        }

        return true;
    }

    const std::array<int, 2>&
    GetChildNodes() const
    {
        return m_childNodeIndicies;
    }

    void
    SetChildNode(int bvhNodeIndex, int childIndex)
    {
        ASSERT(0 <= childIndex && childIndex < 2);
        m_childNodeIndicies[childIndex] = bvhNodeIndex;
    }

    void
    SetLeaf(bool isLeaf)
    {
        m_isLeaf = isLeaf;
    }

    bool
    IsLeaf() const
    {
        return m_isLeaf;
    }

    int
    GetIndexBegin() const
    {
        return m_indexBegin;
    }

    int
    GetIndexEnd() const
    {
        return m_indexEnd;
    }

    const Bounds&
    GetBounds() const
    {
        return m_bounds;
    }

private:
    Bounds m_bounds{};
    std::array<int, 2> m_childNodeIndicies{ -1, -1 };
    int m_indexBegin = -1;
    int m_indexEnd = -1;
    bool m_isLeaf = false;
};

} // namespace Core
} // namespace Petrichor
