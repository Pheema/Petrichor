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
GGX::BxDF(const Ray& rayIn,
          const Ray& rayOut,
          const ShadingInfo& shadingInfo) const
{
    const Math::Vector3f normal = GetNormal(shadingInfo);

    const auto halfVec = (-rayIn.dir + rayOut.dir).Normalized();
    const float hDotN = std::abs(Dot(halfVec, normal));

    const float alpha = GetAlpha(shadingInfo);
    const float alpha2 = alpha * alpha;
    const float hDotN2 = hDotN * hDotN;
    const float k = (alpha2 - 1.0f) * hDotN2 + 1.0f;

    // D
    const float dTerm = alpha2 / (Math::kPi * k * k);

    // G
    const float lambdaIn = Lambda(rayOut.dir, halfVec, alpha);
    const float lambdaOut = Lambda(-rayIn.dir, halfVec, alpha);
    const float gTerm = 1.0f / (1.0f + lambdaIn + lambdaOut);

    // F
    const float hDotL = std::abs(Dot(halfVec, rayOut.dir));
    const float oneMinusCos = 1.0f - hDotL;
    const float oneMinusCos2 = oneMinusCos * oneMinusCos;
    const float oneMinusCos5 = oneMinusCos2 * oneMinusCos2 * oneMinusCos;
    const auto fTerm = m_f0 + (Color3f::One() - m_f0) * oneMinusCos5;

    const float lDotN = std::abs(Dot(rayOut.dir, normal));
    const float vDotN = std::abs(Dot(-rayIn.dir, normal));

    auto f = dTerm * gTerm * fTerm / (4.0f * lDotN * vDotN);

    return f;
}

float
GGX::PDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const
{
    const Math::Vector3f normal = GetNormal(shadingInfo);

    const auto halfVec = (-rayIn.dir + rayOut.dir).Normalized();
    const float hDotN = std::abs(Dot(halfVec, normal));

    const float alpha = GetAlpha(shadingInfo);
    const float alpha2 = alpha * alpha;
    const float hDotN2 = hDotN * hDotN;

    // D
    const float tmp = 1.0f - (1.0f - alpha2) * hDotN2;
    return tmp > 0 ? Math::kInvPi * alpha2 / (tmp * tmp) : kInfinity;
}

Ray
GGX::CreateNextRay(const Ray& rayIn,
                   const ShadingInfo& shadingInfo,
                   ISampler2D& sampler2D) const
{
    const Math::Vector3f normal0 = GetNormal(shadingInfo);
    const float hitSign = -Math::Dot(rayIn.dir, normal0);
    const Math::Vector3f normal = normal0 * std::copysign(1.0f, hitSign);

#if 1 // USE_VNDF_SAMPLING
    Math::Vector3f sampledHalfVec =
      SampleGGXVNDF(-rayIn.dir, shadingInfo, sampler2D);
    // sampledHalfVec = hitInfo.normal;
    Math::Vector3f outDir = (rayIn.dir).Reflected(sampledHalfVec);

    // TODO: あとでFresnel()にまとめる
    const float hDotL = std::abs(Math::Dot(sampledHalfVec, outDir));
    const float oneMinusCos = 1.0f - hDotL;
    const float oneMinusCos2 = oneMinusCos * oneMinusCos;
    const float oneMinusCos5 = oneMinusCos2 * oneMinusCos2 * oneMinusCos;
    const auto fTerm = m_f0 + (Color3f::One() - m_f0) * oneMinusCos5;

    Ray ray(shadingInfo.pos,
            outDir,
            RayTypes::Glossy,
            rayIn.throughput,
            rayIn.bounce + 1);

    const float alpha = GetAlpha(shadingInfo);
    const float lambdaIn = Lambda(outDir, sampledHalfVec, alpha);
    const float lambdaOut = Lambda(-rayIn.dir, sampledHalfVec, alpha);
    const float g2 = 1.0f / (1.0f + lambdaIn + lambdaOut);
    const float g1 = 1.0f / (1.0f + lambdaIn);

    ray.throughput *= (fTerm * g2 / g1);
    ASSERT(ray.throughput.MinElem() >= 0.0f);

#else
    Math::OrthonormalBasis onb;
    onb.Build(normal);

    auto pointSampled = sampler2D.Next();

    float theta = acos(std::get<0>(pointSampled));
    float phi = 2.0f * M_PI * std::get<1>(pointSampled);
    *pdfDir = 1.0f / (2.0f * M_PI);

    auto outDir = onb.GetDir(theta, phi);

    Ray ray(shadingInfo.pos,
            outDir,
            RayTypes::Glossy,
            rayIn.throughput,
            rayIn.bounce + 1);

    auto f = BxDF(rayIn, ray, shadingInfo);
    auto cos = std::max(0.0f, Math::Dot(ray.dir, normal));
    ray.throughput *= (f * cos / *pdfDir);

#endif

    ray.o += kEps * normal;
    return ray;
}

MaterialTypes
GGX::GetMaterialType(const MaterialBase** mat0 /*= nullptr*/,
                     const MaterialBase** mat1 /*= nullptr*/,
                     float* mix /*= nullptr*/) const
{
    return MaterialTypes::GGX;
}

float
GGX::Lambda(const Math::Vector3f& dir,
            const Math::Vector3f& halfDir,
            float alpha) const
{
    const float alpha2 = alpha * alpha;
    const float cos = std::abs(Dot(dir, halfDir));
    const float tan2 = cos > 0 ? 1.0f / (cos * cos) - 1.0f : kInfinity;

    float lambda = -0.5f + 0.5f * sqrt(1.0f + alpha2 * tan2);
    ASSERT(!std::isinf(lambda));
    return lambda;
}
Math::Vector3f
GGX::SampleGGXVNDF(const Math::Vector3f& dirView,
                   const ShadingInfo& shadingInfo,
                   ISampler2D& rng2D) const
{
    const Math::Vector3f normal0 = GetNormal(shadingInfo);
    const float hitSign = Math::Dot(dirView, normal0);
    const Math::Vector3f normal = normal0 * std::copysign(1.0f, hitSign);

    // ---- vを算出 ----
    Math::OrthonormalBasis onbOnSurface;
    const auto t = (std::abs(normal.z) < 0.9999f)
                     ? Cross(normal, Math::Vector3f::UnitZ())
                     : Math::Vector3f::UnitX();

    onbOnSurface.Build(normal, t);

    auto v = onbOnSurface.WorldToLocal(dirView);
    const float alpha = GetAlpha(shadingInfo);
    v *= Math::Vector3f(alpha, alpha, 1.0f);
    v.Normalize();

    // ---- サンプリング ----
    Math::OrthonormalBasis onb;
    const auto t1 = (v.z < 0.9999f)
                      ? Cross(v, Math::Vector3f::UnitZ()).Normalized()
                      : Math::Vector3f::UnitX();
    const auto t2 = Cross(t1, v);
    onb.Build(t2, t1);

    const auto rand = rng2D.Next();
    float u0 = std::get<0>(rand);
    float u1 = std::get<1>(rand);

    float a = 1.0f / (1.0f + v.z);

    float r = sqrt(u0);
    float phi = (u1 < a) ? u1 / a * Math::kPi
                         : Math::kPi + (u1 - a) / (1.0f - a) * Math::kPi;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u1 < a) ? 1.0f : v.z);

    Math::Vector3f n =
      p1 * t1 + p2 * t2 + sqrt(std::max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;

    // ---- unstretch ----
    n = Math::Vector3f(alpha * n.x, alpha * n.y, std::max(0.0f, n.z));

    // ---- local normal to world normal ----
    n = onbOnSurface.LocalToWorld(n);
    n.Normalize();

    ASSERT(std::isfinite(n.x));
    return n;
}

} // namespace Core
} // namespace Petrichor
