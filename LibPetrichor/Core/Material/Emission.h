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

    virtual Color3f
    Radiance(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const override;

    virtual Color3f
    BRDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const override;

    virtual Ray
    CreateNextRay(const Ray& rayIn, const HitInfo& hitInfo, ISampler2D& sampler2D, float* pdfDir) const override;

    virtual MaterialTypes GetMaterialType(const MaterialBase** mat0 = nullptr, const MaterialBase** mat1 = nullptr, float* mix = nullptr) const override;

private:
    Color3f m_color;

};

}   // namespace Core
}   // namespace Petrichor
