#pragma once

#include "Core/Texture2D.h"
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

    //! ラフネス用のテクスチャを指定する
    void
    SetRoughnessTexture(const Texture2D* roughnessTexture)
    {
        m_roughnessTexture = roughnessTexture;
    }

    //! ラフネス用のテクスチャを取得する
    const float
    GetAlpha(const ShadingInfo& shadingInfo) const
    {
        float alpha = 0.0f;
        if (m_roughnessTexture)
        {
            const float roughness =
              GetLuminance(m_roughnessTexture->GetPixelByUV(
                shadingInfo.uv.x,
                shadingInfo.uv.y,
                Texture2D::InterplationTypes::Bilinear));
            alpha = roughness * roughness;
        }
        else
        {
            alpha = m_alpha;
        }
        return alpha;
    }

private:
    float
    Lambda(const Math::Vector3f& dir,
           const Math::Vector3f& halfDir,
           float alpha) const;

    Math::Vector3f
    SampleGGXVNDF(const Math::Vector3f& dirView,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& rng2D) const;

    Color3f m_f0 = Color3f::One(); //!< 垂直入射時の反射色
    float m_alpha = 0.0f;          //!< Roughness^2

    const Texture2D* m_roughnessTexture = nullptr; //!< Roughnessテクスチャ
};

} // namespace Core
} // namespace Petrichor
