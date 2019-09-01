#pragma once

#include "MaterialBase.h"

namespace Petrichor
{
namespace Core
{

class Glass : public MaterialBase
{
public:
    //! 屈折率
    struct IOR
    {
        static constexpr float Water = 1.333f; //!< 水(30degC)
    };

public:
    Glass() = default;

    explicit Glass(const Color3f& color, float ior) noexcept
      : m_color(color)
      , m_ior(ior)
    {
    }

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
    GetMaterialType() const override
    {
        return MaterialTypes::Glass;
    }

    //! ラフネスの2乗値を取得する
    const float
    GetAlpha(const ShadingInfo& shadingInfo) const
    {
        ASSERT(m_alpha >= 0.0f);
        return m_alpha;
    }

private:
    Color3f m_color = Color3f::One();
    float m_ior = IOR::Water;
    float m_alpha = 0.0f; //!< Roughnessの2乗
};

} // namespace Core
} // namespace Petrichor
