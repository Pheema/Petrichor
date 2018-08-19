﻿#pragma once

#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class Texture2D;

class Lambert : public MaterialBase
{
public:
    explicit Lambert(const Color3f& m_kd);

    Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const override;

    float
    PDF(const Ray& rayIn,
        const Ray& rayOut,
        const ShadingInfo& shadingInfo) const override;

    Ray
    CreateNextRay(const Ray& rayIn,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& sampler2D) const override;

    MaterialTypes
    GetMaterialType(const MaterialBase** mat0 = nullptr,
                    const MaterialBase** mat1 = nullptr,
                    float* mix = nullptr) const override;

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
