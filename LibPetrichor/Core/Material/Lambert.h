#pragma once

#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class Texture2D;

class Lambert : public MaterialBase
{
public:
    Lambert(const Color3f& m_kd);

    Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const HitInfo& hitInfo) const override;

    float
    PDF(const Ray& rayIn,
        const Ray& rayOut,
        const HitInfo& hitInfo) const override;

    Ray
    CreateNextRay(const Ray& rayIn,
                  const HitInfo& hitInfo,
                  ISampler2D& sampler2D,
                  float* pdfDir) const override;

    MaterialTypes
    GetMaterialType(const MaterialBase** mat0 = nullptr,
                    const MaterialBase** mat1 = nullptr,
                    float* mix                = nullptr) const override;

    void
    SetTexAlbedo(const Texture2D* texAlbedo)
    {
        m_texAlbedo = texAlbedo;
    }

private:
    Color3f m_kd; // 拡散係数
    const Texture2D* m_texAlbedo = nullptr;
};

} // namespace Core
} // namespace Petrichor
