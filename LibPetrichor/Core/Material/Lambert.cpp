#include "Lambert.h"

#include <Core/Constants.h>
#include <Core/HitInfo.h>
#include <Core/Ray.h>
#include <Core/Texture2D.h>
#include <Core/Sampler/ISampler2D.h>
#include <Math/OrthonormalBasis.h>
#include <Random/XorShift.h>

namespace Petrichor
{
namespace Core
{

Lambert::Lambert(const Color3f& m_kd) :
    m_kd(m_kd)
{
}

Color3f Lambert::BRDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
{
    if (m_texAlbedo != nullptr)
    {
        return m_kd * m_texAlbedo->GetPixelByUV(hitInfo.uv.x, hitInfo.uv.y) * Math::kInvPi;
    }

    return m_kd * Math::kInvPi;
}

float
Lambert::PDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
{
    // Importance sampling
    float cos = abs(Math::Dot(rayOut.dir, hitInfo.normal));
    return cos / (2.0f * Math::kPi);
}

Ray
Lambert::CreateNextRay(const Ray& rayIn, const HitInfo& hitInfo, ISampler2D& sampler2D, float* pdfDir) const
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

    Math::OrthonormalBasis onb;
    onb.Build(normal);

    auto pointSampled = sampler2D.SampleNext2D();

#if 1
    // ImportanceSampling
    float theta = asin(sqrt(std::get<0>(pointSampled)));
    float phi = 2.0f * Math::kPi * std::get<1>(pointSampled);
 
    auto outDir = onb.GetDir(theta, phi);

    Ray ray(
        hitInfo.pos,
        outDir,
        RayTypes::Diffuse,
        rayIn.weight,
        rayIn.bounce + 1
    );

    float cos = abs(Math::Dot(ray.dir, normal));
    *pdfDir = cos / (2.0f * Math::kPi);
    
    ray.weight *= m_kd;
    if (m_texAlbedo != nullptr)
    {
        ray.weight *= m_texAlbedo->GetPixelByUV(hitInfo.uv.x, hitInfo.uv.y);
    }

#else
    float theta = acos(std::get<0>(pointSampled));
    float phi = 2.0f * M_PI * std::get<1>(pointSampled);
    *pdfDir = 1.0f / (2.0f * M_PI);

    auto outDir = onb.GetDir(theta, phi);

    Ray ray(
            hitInfo.pos,
            outDir,
            RayTypes::Diffuse,
            rayIn.weight,
            rayIn.bounce + 1
        );

    auto f = BRDF(rayIn, ray, hitInfo);
    auto cos = std::max(0.0f, Math::Dot(ray.dir, normal));
    ray.weight *= (f * cos / *pdfDir);
#endif
    return ray;
}

Petrichor::Core::MaterialTypes Lambert::GetMaterialType(const MaterialBase** mat0 /*= nullptr*/, const MaterialBase** mat1 /*= nullptr*/, float* mix /*= nullptr*/) const
{
    return MaterialTypes::Lambert;
}

}   // namespace Core
}   // namespace Petrichor
