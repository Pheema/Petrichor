#pragma once

#include "Bound.h"
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
    BVHNode();

    BVHNode(const Math::Vector3f& vMin, const Math::Vector3f& vMax);

    std::optional<HitInfo>
    Intersect(const Ray& ray) const;

    size_t
    Partition(int axis);

    void
    AppendChild(const GeometryBase* geometry);

    const std::vector<const GeometryBase*>&
    GetChildArray() const;

    size_t
    GetNumChildGeoms() const;

    void
    SetChildNode(unsigned nodeSlot, size_t nodeIndex);

    void
    ReserveChildArray(size_t size);

    const std::array<size_t, 2>&
    GetChildNodeIndicies() const;

    void
    SortByNthElement(int widestAxis, float center, size_t nth);

    void
    SetLeaf(bool isLeaf);

    bool
    IsLeaf() const;

    void
    ClearAndShrink()
    {
        m_childGeometries.clear();
        m_childGeometries.shrink_to_fit();
    }

    Bound bound;

private:
    bool m_isLeaf = false;
    std::vector<const GeometryBase*> m_childGeometries;
    std::array<size_t, 2> m_childNodeIndicies{ 0, 0 };
};

#pragma region Inline functions

inline void
BVHNode::AppendChild(const GeometryBase* geometry)
{
    m_childGeometries.emplace_back(geometry);
}

inline const std::vector<const GeometryBase*>&
BVHNode::GetChildArray() const
{
    return m_childGeometries;
}

inline size_t
BVHNode::GetNumChildGeoms() const
{
    return m_childGeometries.size();
}

inline void
BVHNode::SetChildNode(unsigned nodeSlot, size_t nodeIndex)
{
    ASSERT(nodeSlot < 2);
    m_childNodeIndicies[nodeSlot] = nodeIndex;
}

inline void
BVHNode::ReserveChildArray(size_t size)
{
    m_childGeometries.reserve(size);
}

inline const std::array<size_t, 2>&
BVHNode::GetChildNodeIndicies() const
{
    return m_childNodeIndicies;
}

inline void
BVHNode::SetLeaf(bool isLeaf)
{
    m_isLeaf = isLeaf;
}

inline bool
BVHNode::IsLeaf() const
{
    return m_isLeaf;
}

#pragma endregion

} // namespace Core
} // namespace Petrichor
