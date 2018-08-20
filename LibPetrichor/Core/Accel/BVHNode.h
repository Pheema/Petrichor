#pragma once

#include "Bounds.h"
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
    Intersect(const Ray& ray) const;

    bool
    Contains(const Math::Vector3f& point) const;

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
