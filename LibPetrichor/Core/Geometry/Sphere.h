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

    virtual void
    SampleSurface(Math::Vector3f p,
                  ISampler2D& sampler2D,
                  PointData* pointData,
                  float* pdfArea) const override;

    Math::Vector3f o;
    float r;
};

} // namespace Core
} // namespace Petrichor
