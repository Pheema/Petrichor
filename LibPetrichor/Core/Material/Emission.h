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
    Radiance(const Ray& rayIn,
             const Ray& rayOut,
             const HitInfo& hitInfo) const override;

    Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const override;

    Ray
    CreateNextRay(const Ray& rayIn,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& sampler2D,
                  float* pdfDir) const override;

    MaterialTypes
    GetMaterialType(const MaterialBase** mat0 = nullptr,
                    const MaterialBase** mat1 = nullptr,
                    float* mix = nullptr) const override;

private:
    Color3f m_color;
};

} // namespace Core
} // namespace Petrichor
