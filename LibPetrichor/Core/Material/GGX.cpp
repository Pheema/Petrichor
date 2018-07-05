#include "GGX.h"

#include "Core/Constants.h"
#include "Core/HitInfo.h"
#include "Core/Ray.h"
#include "Core/Sampler/ISampler2D.h"
#include "Math/Halton.h"
#include "Math/OrthonormalBasis.h"
#include "Random/XorShift.h"

namespace Petrichor
{
namespace Core
{

GGX::GGX(const Color3f& f0, float roughness)
  : m_f0(f0)
  , m_alpha(roughness * roughness)
{
}

Petrichor::Color3f
GGX::BxDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
{
    const auto halfVec = (-rayIn.dir + rayOut.dir).Normalized();
    const float hDotN  = abs(Dot(halfVec, hitInfo.normal));

    const float alpha2 = m_alpha * m_alpha;
    const float hDotN2 = hDotN * hDotN;
    const float tan2   = (1.0f / hDotN2) - 1.0f;
    const float k      = (alpha2 - 1.0f) * hDotN2 + 1.0f;

    // D
    const float dTerm = alpha2 / (Math::kPi * k * k);

    // G
    const float lambdaIn  = Lambda(rayOut.dir, halfVec);
    const float lambdaOut = Lambda(-rayIn.dir, halfVec);
    const float gTerm     = 1.0f / (1.0f + lambdaIn + lambdaOut);

    // F
    const float hDotL        = abs(Dot(halfVec, rayOut.dir));
    const float oneMinusCos  = 1.0f - hDotL;
    const float oneMinusCos2 = oneMinusCos * oneMinusCos;
    const float oneMinusCos5 = oneMinusCos2 * oneMinusCos2 * oneMinusCos;
    const auto fTerm         = m_f0 + (Color3f::One() - m_f0) * oneMinusCos5;

    const float lDotN = abs(Dot(rayOut.dir, hitInfo.normal));
    const float vDotN = abs(Dot(-rayIn.dir, hitInfo.normal));

    auto f = dTerm * gTerm * fTerm / (4.0f * lDotN * vDotN);

    return f;
}

float
GGX::PDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
{
    const auto halfVec = (-rayIn.dir + rayOut.dir).Normalized();
    const float hDotN  = abs(Dot(halfVec, hitInfo.normal));

    const float alpha2 = m_alpha * m_alpha;
    const float hDotN2 = hDotN * hDotN;
    const float tan2   = (1.0f / hDotN2) - 1.0f;
    const float k      = 1.0f + tan2 / alpha2;

    // D
    const float dTerm = 1.0f / (Math::kPi * alpha2 * hDotN2 * hDotN2 * k * k);

    // TODO: nanチェックが雑
    if (hDotN2 < 1e-4f)
    {
        return kInfinity;
    }

    return dTerm; // * g1 / (4.0f * abs(Math::Dot(-rayIn.dir, hitInfo.normal)));
}

Ray
GGX::CreateNextRay(const Ray& rayIn,
                   const HitInfo& hitInfo,
                   ISampler2D& rng2D,
                   float* pdfDir) const
{
    Math::Vector3f normal;

    if (Math::Dot(rayIn.dir, hitInfo.normal) >= 0)
    {
        normal = -hitInfo.normal;
    }
    else
    {
        normal = hitInfo.normal;
    }

#if 1 // USE_VNDF_SAMPLING
    Math::Vector3f sampledHalfVec = SampleGGXVNDF(-rayIn.dir, hitInfo, rng2D);
    // sampledHalfVec = hitInfo.normal;
    Math::Vector3f outDir = (rayIn.dir).Reflected(sampledHalfVec);

    /*std::cout << "[N]: " << hitInfo.normal << std::endl;
std::cout << "[H]" << sampledHalfVec << std::endl;
std::cout << "[I]" << rayIn.dir << std::endl;
std::cout << "[O]" << outDir << std::endl;*/

    // TODO: あとでFresnel()にまとめる
    const float hDotL        = abs(Math::Dot(sampledHalfVec, outDir));
    const float oneMinusCos  = 1.0f - hDotL;
    const float oneMinusCos2 = oneMinusCos * oneMinusCos;
    const float oneMinusCos5 = oneMinusCos2 * oneMinusCos2 * oneMinusCos;
    const auto fTerm         = m_f0 + (Color3f::One() - m_f0) * oneMinusCos5;

    Ray ray(
      hitInfo.pos, outDir, RayTypes::Glossy, rayIn.weight, rayIn.bounce + 1);

    ASSERT(std::isfinite(ray.dir.x));

    const float lambdaIn  = Lambda(outDir, sampledHalfVec);
    const float lambdaOut = Lambda(-rayIn.dir, sampledHalfVec);
    const float g2        = 1.0f / (1.0f + lambdaIn + lambdaOut);
    const float g1        = 1.0f / (1.0f + lambdaIn);

    *pdfDir = PDF(rayIn, ray, hitInfo);
    ray.weight *= (fTerm * g2 / g1);

#else
    Math::OrthonormalBasis onb;
    onb.Build(normal);

    auto pointSampled = rng2D.Next();

    float theta = acos(std::get<0>(pointSampled));
    float phi   = 2.0f * M_PI * std::get<1>(pointSampled);
    *pdfDir     = 1.0f / (2.0f * M_PI);

    auto outDir = onb.GetDir(theta, phi);

    Ray ray(
      hitInfo.pos, outDir, RayTypes::Glossy, rayIn.weight, rayIn.bounce + 1);

    auto f   = BxDF(rayIn, ray, hitInfo);
    auto cos = std::max(0.0f, Math::Dot(ray.dir, normal));
    ray.weight *= (f * cos / *pdfDir);

#endif
    return ray;
}

Petrichor::Core::MaterialTypes
GGX::GetMaterialType(const MaterialBase** mat0 /*= nullptr*/,
                     const MaterialBase** mat1 /*= nullptr*/,
                     float* mix /*= nullptr*/) const
{
    return MaterialTypes::GGX;
}

float
GGX::Lambda(const Math::Vector3f& dir, const Math::Vector3f& halfDir) const
{
    const float alpha2 = m_alpha * m_alpha;
    const float cos    = abs(Dot(dir, halfDir));
    const float tan2   = 1.0f / (cos * cos) - 1.0f;

    float lambda = -0.5f + 0.5f * sqrt(1.0f + alpha2 * tan2);
    return lambda;
}

Math::Vector3f
GGX::SampleGGXVNDF(const Math::Vector3f& dirView,
                   const HitInfo& hitInfo,
                   ISampler2D& rng2D) const
{
    Math::Vector3f normal;

    if (Math::Dot(dirView, hitInfo.normal) >= 0)
    {
        normal = hitInfo.normal;
    }
    else
    {
        normal = -hitInfo.normal;
    }

    // ---- vを算出 ----
    Math::OrthonormalBasis onbOnSurface;
    const auto t = (std::abs(normal.z) < 0.9999f)
                     ? Cross(normal, Math::Vector3f::UnitZ())
                     : Math::Vector3f::UnitX();

    onbOnSurface.Build(normal, t);

    auto v = onbOnSurface.WorldToLocal(dirView);
    v *= Math::Vector3f(m_alpha, m_alpha, 1.0f);
    v.Normalize();

    // ---- サンプリング ----
    Math::OrthonormalBasis onb;
    const auto t1 = (v.z < 0.9999f)
                      ? Cross(v, Math::Vector3f::UnitZ()).Normalized()
                      : Math::Vector3f::UnitX();
    const auto t2 = Cross(t1, v);
    onb.Build(t2, t1);

    const auto rand = rng2D.Next();
    float u0        = std::get<0>(rand);
    float u1        = std::get<1>(rand);

    float a = 1.0f / (1.0f + v.z);

    float r   = sqrt(u0);
    float phi = (u1 < a) ? u1 / a * Math::kPi
                         : Math::kPi + (u1 - a) / (1.0f - a) * Math::kPi;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u1 < a) ? 1.0f : v.z);

    Math::Vector3f n =
      p1 * t1 + p2 * t2 + sqrt(std::max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;

    // ---- unstretch ----
    n = Math::Vector3f(m_alpha * n.x, m_alpha * n.y, std::max(0.0f, n.z));

    // ---- local normal to world normal ----
    n = onbOnSurface.LocalToWorld(n);
    n.Normalize();

    ASSERT(std::isfinite(n.x));
    return n;
}

} // namespace Core
} // namespace Petrichor
