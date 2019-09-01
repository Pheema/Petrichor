#pragma once

#include "Core/HitInfo.h"
#include "Core/Ray.h"
#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class ISampler2D;

class MixMaterial : public MaterialBase
{
public:
    MixMaterial(const MaterialBase* mat0, const MaterialBase* mat1, float mix);

    Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const override
    {
        ASSERT(false);
        return Color3f();
    }

    Ray
    CreateNextRay(const Ray& rayIn,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& sampler2D) const override
    {
        ASSERT(false);
        return Ray();
    }

    MaterialTypes
    GetMaterialType() const override
    {
        return MaterialTypes::Mix;
    }

    //! ミックスされた2つのマテリアルから1つを取り出す
    //! @param rand [0, 1]の乱数
    const MaterialBase*
    GetSingleMaterial(float rand) const
    {
        return m_mix <= rand ? m_mat0 : m_mat1;
    }

private:
    const MaterialBase* m_mat0 = nullptr;
    const MaterialBase* m_mat1 = nullptr;
    float m_mix = 0.0f;
};

} // namespace Core
} // namespace Petrichor
