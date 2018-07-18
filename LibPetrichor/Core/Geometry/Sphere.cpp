#include "Sphere.h"

#include "Core/Constants.h"
#include "Core/HitInfo.h"
#include "Core/Ray.h"
#include "Core/Sampler/ISampler2D.h"
#include "Math/OrthonormalBasis.h"
#include <tuple>

namespace Petrichor
{
namespace Core
{

using namespace Math;

Sphere::Sphere()
  : m_origin(Vector3f::Zero())
  , m_radius(0.0f)
{
    // Do nothing
}

Sphere::Sphere(const Math::Vector3f& o, float r)
  : m_origin(o)
  , m_radius(r)
{
    // Do nothing
}

void
Sphere::CalcBound() const
{
    const auto vMin = m_origin - Math::Vector3f::One() * m_radius;
    const auto vMax = m_origin + Math::Vector3f::One() * m_radius;
    m_bound = Bound(vMin, vMax);
}

std::optional<HitInfo>
Sphere::Intersect(const Ray& ray) const
{
    const float a = Dot(ray.dir, ray.dir);
    const float b_2 = Dot(ray.dir, ray.o - m_origin);
    const float c =
      Dot(ray.o - m_origin, ray.o - m_origin) - m_radius * m_radius;
    const float D_4 = b_2 * b_2 - a * c;
    if (D_4 < 0.0f)
    {
        // レイの進行する上に球面が存在しない
        return std::nullopt;
    }

    const float l1 = (-b_2 - sqrt(D_4)) / a;
    const float l2 = (-b_2 + sqrt(D_4)) / a;

    if (l1 < 0.0f && l2 < 0.0f)
    {
        // レイの進行後方に球面が存在する場合
        return std::nullopt;
    }

    HitInfo hitInfo;
    hitInfo.distance = std::min(std::max(l1, 0.0f), std::max(l2, 0.0f));
    hitInfo.hitObj = this;
    return hitInfo;
}

ShadingInfo
Sphere::Interpolate(const Ray& ray, const HitInfo& hitInfo) const
{
    ShadingInfo shadingInfo;
    shadingInfo.pos = ray.o + ray.dir * hitInfo.distance;

    const float l2 = (ray.o - m_origin).SquaredLength();

    if (l2 > m_radius * m_radius)
    {
        // 球の外側からの衝突
        shadingInfo.normal = (shadingInfo.pos - m_origin).Normalized();
    }
    else
    {
        // 球内部からの衝突
        shadingInfo.normal = -(shadingInfo.pos - m_origin).Normalized();
    }

    // #TODO: UV

    return shadingInfo;
}

void
Sphere::SampleSurface(Math::Vector3f p,
                      ISampler2D& sampler2D,
                      PointData* pointData,
                      float* pdfArea) const
{

    Math::OrthonormalBasis onb;

    Math::Vector3f originToPoint = p - m_origin;

    onb.Build(originToPoint);

    auto pointSampled = sampler2D.Next();

    float l = originToPoint.Length();
    // TODO: 点pが球の中にある場合に破綻するのでチェック
    ASSERT(l >= m_radius);

    float theta =
      acos(1.0f - std::get<0>(pointSampled) * (1.0f - m_radius / l));
    float phi = 2.0f * Math::kPi * std::get<1>(pointSampled);

    auto dir = onb.GetDir(theta, phi);
    auto pointOnSurface = m_origin + m_radius * dir;

    pointData->pos = pointOnSurface;
    pointData->normal = dir;
    *pdfArea = l / (2.0f * Math::kPi * m_radius * m_radius * (l - m_radius));
}

} // namespace Core
} // namespace Petrichor
