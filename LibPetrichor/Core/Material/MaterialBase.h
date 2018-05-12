﻿#pragma once

#include "Core/Color3f.h"

namespace Petrichor
{
namespace Core
{

struct Ray;
struct HitInfo;
class ISampler2D;

enum class MaterialTypes
{
    Lambert,
    Emission,
    GGX,
    Mix
};

class MaterialBase
{
public:
    virtual Color3f
    Radiance(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
    {
        ASSERT(false);
        return Color3f();
    };

    virtual Color3f
    BRDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const = 0;

    virtual float
    PDF(const Ray& rayIn, const Ray& rayOut, const HitInfo& hitInfo) const
    {
        ASSERT(false);
        return 0.0f;
    };

    virtual Ray
    CreateNextRay(const Ray& rayIn,
                  const HitInfo& hitInfo,
                  ISampler2D& sampler2D,
                  float* pdfDir) const = 0;

    virtual MaterialTypes
    GetMaterialType(const MaterialBase** mat0 = nullptr,
                    const MaterialBase** mat1 = nullptr,
                    float* mix                = nullptr) const = 0;
};

} // namespace Core
} // namespace Petrichor
