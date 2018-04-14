#include "Sphere.h"

#include <tuple>
#include <Core/Ray.h>
#include <Core/HitInfo.h>
#include <Core/Sampler/ISampler2D.h>
#include <Math/OrthonormalBasis.h>

namespace Petrichor
{
namespace Core
{

using namespace Math;

Sphere::Sphere() :
    o(Vector3f::Zero()),
    r(0.0f)
{
    // Do nothing
}

Sphere::Sphere(const Math::Vector3f& o, float r) :
    o(o),
    r(r)
{
    // Do nothing
}

Bound
Sphere::CalcBound() const
{
    const auto vMin = o - Math::Vector3f::One() * r;
    const auto vMax = o + Math::Vector3f::One() * r;

    Bound bound(vMin, vMax);
    m_bound = bound;

    return m_bound;
}


bool
Sphere::Intersect(const Ray& ray, HitInfo* hitInfo) const
{
    const float a = Dot(ray.dir, ray.dir);
    const float b_2 = Dot(ray.dir, ray.o - o);
    const float c = Dot(ray.o - o, ray.o - o) - r * r;
    const float D_4 = b_2 * b_2 - a * c;
    if (D_4 < 0.0f)
    {
        // レイの進行する上に球面が存在しない
        return false;
    }

    const float l1 = (-b_2 - sqrt(D_4)) / a;
    const float l2 = (-b_2 + sqrt(D_4)) / a;

    if (l1 < 0.0f && l2 < 0.0f)
    {
        // レイの進行後方に球面が存在する場合
        return false;
    }

    float distance = std::min(std::max(l1, 0.0f), std::max(l2, 0.0f));

    hitInfo->distance = distance;
    hitInfo->pos = ray.o + ray.dir * hitInfo->distance;

    if (Math::ApproxEq(hitInfo->distance, l1))
    {
        // 球の外側からの衝突
        hitInfo->normal = (hitInfo->pos - o).Normalized();
    }
    else
    {
        // 球内部からの衝突
        hitInfo->normal = -(hitInfo->pos - o).Normalized();
    }

    hitInfo->hitObj = this;

    return true;
}

void 
Sphere::SampleSurface(Math::Vector3f p, ISampler2D& sampler2D, PointData* pointData, float* pdfArea) const
{
    
    Math::OrthonormalBasis onb;

    Math::Vector3f originToPoint = p - o;

    onb.Build(originToPoint);

    auto pointSampled = sampler2D.SampleNext2D();

    float l = originToPoint.Length();
    // TODO: 点pが球の中にある場合に破綻するのでチェック
    ASSERT(l >= r);

    float theta = acos(1.0f - std::get<0>(pointSampled) * (1.0f - r / l));
    float phi = 2.0f * M_PI * std::get<1>(pointSampled);

    auto dir = onb.GetDir(theta, phi);
    auto pointOnSurface = o + r * dir;

    pointData->pos = pointOnSurface;
    pointData->normal = dir;
    *pdfArea = l / (2.0f * M_PI * r * r * (l - r));
}

}   // namespace Core
}   // namespace Petrichor
