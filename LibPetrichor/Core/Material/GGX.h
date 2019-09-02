#pragma once

#include "Core/Texture2D.h"
#include "MaterialBase.h"
#include "Math/OrthonormalBasis.h"
#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

class ISampler2D;
struct HitInfo;

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
    GetMaterialType() const override
    {
        return MaterialTypes::Glossy;
    }

    //! ラフネス用のテクスチャを指定する
    void
    SetRoughnessMap(const Texture2D* roughnessMap)
    {
        m_roughnessMap = roughnessMap;
    }

    //! ノーマルマップを指定
    void
    SetNormalMap(const Texture2D* normalMap)
    {
        m_normalMap = normalMap;
    }

    //! ラフネス用のテクスチャを取得する
    const float
    GetAlpha(const ShadingInfo& shadingInfo) const
    {
        ASSERT(m_alpha >= 0.0f);

        if (m_roughnessMap)
        {
            const float roughness = m_roughnessMapStrength *
                                    GetLuminance(m_roughnessMap->GetPixelByUV(
                                      shadingInfo.uv.x,
                                      shadingInfo.uv.y,
                                      Texture2D::InterplationTypes::Bilinear));
            return roughness * roughness;
        }
        else
        {
            return m_alpha;
        }
    }

    void
    SetF0Texture(const Texture2D* texture)
    {
        m_reflectanceMap = texture;
    }

    void
    SetNormalMapStrength(float strength)
    {
        m_normalMapStrength = std::clamp(strength, 0.0f, 1.0f);
    }

    void
    SetRoughnessMapStrength(float strength)
    {
        m_roughnessMapStrength = strength;
    }

    Math::Vector3f
    GetNormal(const ShadingInfo& shadingInfo) const
    {
        if (m_normalMap)
        {
            Math::OrthonormalBasis localBasis;
            localBasis.Build(shadingInfo.normal, shadingInfo.tangent);

            const Math::Vector3f normalMap =
              m_normalMap->GetPixelByUV(shadingInfo.uv.x,
                                        shadingInfo.uv.y,
                                        Texture2D::InterplationTypes::Bilinear);

            const Math::Vector3f localNormal =
              (2.0f * normalMap - Math::Vector3f::One()).Normalized();

            return (1.0f - m_normalMapStrength) * localNormal.x *
                     localBasis.GetU() +
                   localNormal.y * localBasis.GetV() +
                   (1.0f - m_normalMapStrength) * localNormal.z *
                     localBasis.GetW();
        }
        else
        {
            return shadingInfo.normal;
        }
    }

    //!< 垂直入射時の反射率
    //!< Ref: "4.8.3.2: Reflectance remapping"
    //!< https://google.github.io/filament/Filament.md.html
    Color3f
    GetF0(const ShadingInfo& shadingInfo) const
    {
        auto reflectance = m_reflectanceMapStrength;

        if (m_reflectanceMap)
        {
            reflectance *= m_reflectanceMap->GetPixelByUV(shadingInfo.uv.x,
                                                          shadingInfo.uv.y);
        }

        return 0.16f * Math::Pow<2>(reflectance) * (1.0f - m_metalness) +
               m_baseColor * m_metalness;
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

private:
    float m_metalness = 0.0f;
    Color3f m_baseColor = Color3f::One();

    float m_alpha = 0.0f; //!< Roughness^2

    Color3f m_reflectanceMapStrength = Color3f::One(); //!< 垂直入射時の反射色
    float m_normalMapStrength = 1.0f; //!< ノーマルマップの強度[0, 1]
    float m_roughnessMapStrength = 1.0f; //!< ノーマルマップの強度[0, 1]

    const Texture2D* m_reflectanceMap = nullptr; //!< 垂直入射時の反射色マップ
    const Texture2D* m_roughnessMap = nullptr; //!< ラフネスマップ
    const Texture2D* m_normalMap = nullptr;    //!< ノーマルマップ
};

} // namespace Core
} // namespace Petrichor
