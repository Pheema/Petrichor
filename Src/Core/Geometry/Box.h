#pragma once

#include <array>

#include <Core/Geometry/GeometryBase.h>

namespace Petrichor
{
namespace Core
{
    struct Vertex;
    struct HitInfo;

    class Triangle : public GeometryBase
    {
    public:
        bool
        Intersect(const Ray& ray, HitInfo* hitInfo) const override;

    protected:
        std::array<const Vertex*, 3> m_verticies;

    };
}   // namespace Core
}   // namespace Petrichor
