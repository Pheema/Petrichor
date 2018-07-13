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
    Sphere();

    Sphere(const Math::Vector3f& o, float r);

    virtual Bound
    CalcBound() const override;

    virtual std::optional<HitInfo>
    Intersect(const Ray& ray) const override;

    virtual ShadingInfo
    Interpolate(const Ray& ray, const HitInfo& hitInfo) const override;

    virtual void
    SampleSurface(Math::Vector3f p,
                  ISampler2D& sampler2D,
                  PointData* pointData,
                  float* pdfArea) const override;

    const Math::Vector3f&
    GetOrigin() const
    {
        return m_origin;
    }

private:
    Math::Vector3f m_origin;
    float m_radius;
};

} // namespace Core
} // namespace Petrichor
