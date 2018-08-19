﻿#pragma once

#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class ISampler2D;
struct HitInfo;
class Math::Vector3f;

class GGX : public MaterialBase
{
public:
    GGX(const Color3f& f0, float roughness = 1.0f);

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

private:
    float
    Lambda(const Math::Vector3f& dir, const Math::Vector3f& halfDir) const;

    Math::Vector3f
    SampleGGXVNDF(const Math::Vector3f& dirView,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& rng2D) const;

    Color3f m_f0;  // 垂直入射時の反射色
    float m_alpha; // ラフネス(X)
};

} // namespace Core
} // namespace Petrichor
