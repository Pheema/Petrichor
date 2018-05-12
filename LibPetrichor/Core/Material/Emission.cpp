#include "Emission.h"

#include "Core/Ray.h"

namespace Petrichor
{
namespace Core
{

Emission::Emission(const Color3f& color)
  : m_color(color)
{
}

Color3f
Emission::Radiance(const Ray& rayIn,
                   const Ray& rayOut,
                   const HitInfo& hitInfo) const
{
    return m_color;
}

Color3f
Emission::BRDF(const Ray& rayIn,
               const Ray& rayOut,
               const HitInfo& hitInfo) const
{
    ASSERT(false);
    return Color3f::Zero();
}

Ray
Emission::CreateNextRay(const Ray& rayIn,
                        const HitInfo& hitInfo,
                        ISampler2D& sampler2D,
                        float* pdfDir) const
{
    ASSERT(false);
    return Ray();
}

Petrichor::Core::MaterialTypes
Emission::GetMaterialType(const MaterialBase** mat0 /*= nullptr*/,
                          const MaterialBase** mat1 /*= nullptr*/,
                          float* mix /*= nullptr*/) const
{
    return MaterialTypes::Emission;
}

} // namespace Core
} // namespace Petrichor
