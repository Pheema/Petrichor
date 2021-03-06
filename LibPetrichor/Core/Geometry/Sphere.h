#pragma once

#include "Core/Geometry/GeometryBase.h"
#include <array>

namespace Petrichor
{
namespace Core
{

class Sphere : public GeometryBase
{
public:
    Sphere() = default;

    Sphere(const Math::Vector3f& o, float r);

    AABB
    CalcBoundary() const override;

    std::optional<HitInfo>
    Intersect(const Ray& ray) const override;

    ShadingInfo
    Interpolate(const Ray& ray, const HitInfo& hitInfo) const override;

    void
    SampleSurface(Math::Vector3f p,
                  ISampler2D& sampler2D,
                  PointData* pointData,
                  float* pdfArea) const override;

    Math::Vector3f
    GetCentroid() const override
    {
        return GetOrigin();
    }

    const Math::Vector3f&
    GetOrigin() const
    {
        return m_origin;
    }

private:
    Math::Vector3f m_origin;
    float m_radius = 1.0f;
};

} // namespace Core
} // namespace Petrichor
