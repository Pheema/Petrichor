#include "Lambert.h"

#include "Core/Constants.h"
#include "Core/HitInfo.h"
#include "Core/Ray.h"
#include "Core/Sampler/ISampler2D.h"
#include "Core/Texture2D.h"
#include "Math/OrthonormalBasis.h"
#include "Random/XorShift.h"

namespace Petrichor
{
namespace Core
{

Lambert::Lambert(const Color3f& m_kd)
  : m_kd(m_kd)
{
}

Color3f
Lambert::BRDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
{
    Color3f albedo = Color3f::One();
    if (m_texAlbedo)
    {
        albedo = m_texAlbedo->GetPixelByUV(hitInfo.uv.x, hitInfo.uv.y);
    }

    return m_kd * albedo * Math::kInvPi;
}

float
Lambert::PDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
{
    if (IsImportanceSamplingEnabled())
    {
        float cos = abs(Math::Dot(rayOut.dir, hitInfo.normal));
        return cos * Math::kInvPi;
    }
    else
    {
        return 0.5f * Math::kInvPi;
    }
}

Ray
Lambert::CreateNextRay(const Ray& rayIn,
                       const HitInfo& hitInfo,
                       ISampler2D& sampler2D,
                       float* pdfDir) const
{
    const float hitSign = -Math::Dot(rayIn.dir, hitInfo.normal);

    const Math::Vector3f normal = hitInfo.normal * std::copysign(1.0f, hitSign);

    Math::OrthonormalBasis onb;
    onb.Build(normal);

    auto[rand0, rand1] = sampler2D.Next();

    if (IsImportanceSamplingEnabled())
    {
        float theta = asin(sqrt(rand0));
        float phi   = 2.0f * Math::kPi * rand1;

        auto outDir = onb.GetDir(theta, phi);
        *pdfDir     = cos(theta) * Math::kInvPi;

        auto outWeight = rayIn.weight * m_kd;
        if (m_texAlbedo)
        {
            outWeight *= m_texAlbedo->GetPixelByUV(hitInfo.uv.x, hitInfo.uv.y);
        }

        return {
            hitInfo.pos, outDir, RayTypes::Diffuse, outWeight, rayIn.bounce + 1
        };
    }
    else
    {
        float theta = acos(rand0);
        float phi   = 2.0f * Math::kPi * rand1;

        auto outDir = onb.GetDir(theta, phi);
        *pdfDir     = 0.5f * Math::kInvPi;

        Ray rayOut(hitInfo.pos,
                   outDir,
                   RayTypes::Diffuse,
                   rayIn.weight,
                   rayIn.bounce + 1);

        auto f = BRDF(rayIn, rayOut, hitInfo);
        rayOut.weight *= (f * cos(theta) / *pdfDir);

        return rayOut;
    }
}

MaterialTypes
Lambert::GetMaterialType(const MaterialBase** mat0 /*= nullptr*/,
                         const MaterialBase** mat1 /*= nullptr*/,
                         float* mix /*= nullptr*/) const
{
    return MaterialTypes::Lambert;
}

} // namespace Core
} // namespace Petrichor
