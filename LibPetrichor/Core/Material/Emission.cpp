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
Emission::BxDF(const Ray& rayIn,
               const Ray& rayOut,
               const ShadingInfo& shadingInfo) const
{
    ASSERT(false);
    return Color3f::Zero();
}

Ray
Emission::CreateNextRay(const Ray& rayIn,
                        const ShadingInfo& shadingInfo,
                        ISampler2D& sampler2D) const
{
    ASSERT(false);
    return Ray();
}

} // namespace Core
} // namespace Petrichor
