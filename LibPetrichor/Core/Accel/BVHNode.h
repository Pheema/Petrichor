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
    struct InternalNodeData
    {
        Bounds bounds;
        std::array<BVHNode*, 2> childNodes;
    };

    struct LeafNodeData
    {
        Bounds bounds;
        uint32_t indexOffset;
        uint32_t numPrimitives;
    };

    BVHNode();

    explicit BVHNode(const InternalNodeData& internalNodeData);

    explicit BVHNode(const LeafNodeData& leafNodeData);

    std::optional<HitInfo>
    Intersect(const Ray& ray) const;

    bool
    Contains(const Math::Vector3f& point) const;

    const std::array<BVHNode*, 2>&
    GetChildNodes() const
    {
        return m_childNodes;
    }

    bool
    IsLeaf() const
    {
        return m_isLeaf;
    }

    uint32_t
    GetIndexOffset() const
    {
        ASSERT(IsLeaf() == false);
        return m_indexOffset;
    }

    uint32_t
    GetNumPrimitives() const
    {
        ASSERT(IsLeaf());
        return m_numPrimitives;
    }

    const Bounds&
    GetBounds() const
    {
        return m_bounds;
    }

private:
    Bounds m_bounds{};
    std::array<BVHNode*, 2> m_childNodes{ 0, 0 };
    uint32_t m_indexOffset = 0;
    uint32_t m_numPrimitives = 0;
    bool m_isLeaf = false;
};

} // namespace Core
} // namespace Petrichor
