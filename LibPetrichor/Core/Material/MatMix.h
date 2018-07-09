#pragma once

#include "Core/HitInfo.h"
#include "Core/Ray.h"
#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class ISampler2D;

class MatMix : public MaterialBase
{
public:
    MatMix(const MaterialBase* mat0, const MaterialBase* mat1, float mix);

    virtual Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const override
    {
        ASSERT(false);
        return Color3f();
    }

    virtual Ray
    CreateNextRay(const Ray& rayIn,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& sampler2D,
                  float* pdfDir) const override
    {
        ASSERT(false);
        return Ray();
    }

    virtual MaterialTypes
    GetMaterialType(const MaterialBase** mat0 = nullptr,
                    const MaterialBase** mat1 = nullptr,
                    float* mix = nullptr) const override;

private:
    const MaterialBase* m_mat0 = nullptr;
    const MaterialBase* m_mat1 = nullptr;
    float m_mix = 0.0f;
};

} // namespace Core
} // namespace Petrichor
