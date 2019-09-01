#pragma once

#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class Emission : public MaterialBase
{
public:
    Emission(const Color3f& color);

    Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const override;

    Ray
    CreateNextRay(const Ray& rayIn,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& sampler2D) const override;

    MaterialTypes
    GetMaterialType() const override
    {
        return MaterialTypes::Emission;
    }

    const Color3f&
    GetLightColor() const
    {
        return m_color;
    }

private:
    Color3f m_color;
};

} // namespace Core
} // namespace Petrichor
