#pragma once

#include <array>
#include <Core/Geometry/GeometryBase.h>

namespace Petrichor
{
namespace Core
{

class Sphere : public GeometryBase
{
public:
#pragma region Public member functions

    Sphere();
    Sphere(const Math::Vector3f& o, float r);

    virtual Bound
    CalcBound() const override;

    virtual bool
    Intersect(const Ray& ray, HitInfo* hitInfo) const override;

    virtual void
    SampleSurface(Math::Vector3f p, ISampler2D& sampler2D, PointData* pointData, float* pdfArea) const override;

#pragma endregion

#pragma region Public member variables

    Math::Vector3f o;
    float r;

#pragma endregion

};

}   // namespace Core
}   // namespace Petrichor
