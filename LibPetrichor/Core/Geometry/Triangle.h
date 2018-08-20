#pragma once

#include "Core/Geometry/GeometryBase.h"
#include "Core/Geometry/Vertex.h"
#include <array>

namespace Petrichor
{
namespace Core
{

struct HitInfo;
struct ShadingInfo;

enum class ShadingTypes
{
    Flat,
    Smooth
};

class Triangle : public GeometryBase
{
public:
    explicit Triangle(ShadingTypes shadingType);

    Triangle(const Vertex* v0, const Vertex* v1, const Vertex* v2);

    Triangle(const Vertex* v0,
             const Vertex* v1,
             const Vertex* v2,
             ShadingTypes shadingType);

    void
    SetVertices(const Vertex* v0, const Vertex* v1, const Vertex* v2);

    Bounds
    CalcBound() const override;

    std::optional<HitInfo>
    Intersect(const Ray& ray) const override;

    ShadingInfo
    Interpolate(const Ray& ray, const HitInfo& hitInfo) const override;

    Math::Vector3f
    GetCentroid() const override
    {
        return (m_vertices[0]->pos + m_vertices[1]->pos + m_vertices[2]->pos) /
               3.0f;
    }

    void
    SampleSurface(Math::Vector3f p,
                  ISampler2D& sampler2D,
                  PointData* pointData,
                  float* pdfArea) const override;

protected:
    std::array<const Vertex*, 3> m_vertices{};
    ShadingTypes m_shadingType = ShadingTypes::Flat;
};

} // namespace Core
} // namespace Petrichor
