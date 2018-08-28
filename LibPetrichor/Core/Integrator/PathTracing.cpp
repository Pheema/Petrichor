#include "PathTracing.h"

#include "Core/Accel/BruteForce.h"
#include "Core/HitInfo.h"
#include "Core/Material/Emission.h"
#include "Core/Sampler/MicroJitteredSampler.h"
#include "Core/Scene.h"
#include "Core/Texture2D.h"
#include <Random/XorShift.h>
#include <algorithm>
#include <sstream>

// #define BALANCE_HEURISTIC

namespace Petrichor
{
namespace Core
{

#pragma optimize("", off)
void
PathTracing::Render(uint32_t pixelX,
                    uint32_t pixelY,
                    const Scene& scene,
                    Texture2D* targetTex,
                    ISampler1D& sampler1D,
                    ISampler2D& sampler2D)
{
    const auto* const mainCamera = scene.GetMainCamera();
    if (mainCamera == nullptr)
    {
        std::cerr << "[Error] Main camera is not found." << std::endl;
        return;
    }

    const uint32_t numSamples = scene.GetSceneSettings().numSamplesPerPixel;
    Color3f pixelColorSum;
    for (uint32_t spp = 0; spp < numSamples; spp++)
    {
        Color3f color;
        Ray ray = mainCamera->PixelToRay(pixelX,
                                         pixelY,
                                         targetTex->GetWidth(),
                                         targetTex->GetHeight(),
                                         sampler2D);
        Ray prevRay = ray;

        ShadingInfo shadingInfo;
        {
            const auto hitInfo = scene.Intersect(ray);
            if (hitInfo == std::nullopt)
            {
                // IBL
                pixelColorSum +=
                  ray.weight * scene.GetEnvironment().GetColor(ray.dir);
                continue;
            }

            shadingInfo = (*hitInfo->hitObj).Interpolate(ray, hitInfo.value());
        }

        // ---- 光源に直接ヒットした場合 ----
        const MaterialBase* mat = shadingInfo.material;
        if (mat->GetMaterialType() == MaterialTypes::Emission)
        {
            auto matLight = static_cast<const Emission*>(mat);
            pixelColorSum += ray.weight * matLight->GetLightColor();
            continue;
        }

        // ---- 光源以外のオブジェクトにヒットした場合 ----
        const uint32_t maxNumBounces = scene.GetSceneSettings().numMaxBouces;
        for (uint32_t bounce = 0; bounce < maxNumBounces; bounce++)
        {
            // ---- ライトをサンプリング ----
            color += CalcLightContribution(
              scene, shadingInfo, sampler1D, sampler2D, ray);

            // 次のレイを生成
            mat = shadingInfo.material;
            ASSERT(mat->GetMaterialType() != MaterialTypes::Emission);
            ASSERT(std::isfinite(ray.dir.x));

            prevRay = ray;
            ray = mat->CreateNextRay(ray, shadingInfo, sampler2D);

            if (Math::ApproxEq(ray.weight.SquaredLength(), 0.0f, kEps))
            {
                break;
            }

            // MIS
            if (const auto hitInfoNext = scene.Intersect(ray); hitInfoNext)
            {
                const ShadingInfo shadingInfoNext =
                  hitInfoNext->hitObj->Interpolate(ray, *hitInfoNext);

                if (shadingInfoNext.material->GetMaterialType() ==
                    MaterialTypes::Emission)
                {
                    float pdfArea = 0.0f;
                    float misWeight = 1.0f;

                    const float l2 =
                      (shadingInfoNext.pos - ray.o).SquaredLength();
                    const float cosP =
                      abs(Math::Dot(ray.dir, shadingInfoNext.normal));

                    // TODO:
                    // 下の関数が確率密度関数を取得しているだけなのに無駄
                    PointData pointData;
                    SampleLight(scene,
                                ray.o,
                                sampler1D.Next(),
                                sampler2D,
                                &pdfArea,
                                nullptr);

                    const float pdfBSDF = mat->PDF(prevRay, ray, shadingInfo);
                    const float pdfLight = l2 / cosP * pdfArea;
#ifdef BALANCE_HEURISTIC
                    misWeight = pdfBSDF / (pdfLight + pdfBSDF);
#else
                    misWeight = pdfBSDF * pdfBSDF /
                                (pdfLight * pdfLight + pdfBSDF * pdfBSDF);
#endif
                    ASSERT(std::isfinite(misWeight) && misWeight >= 0.0f);

                    {
                        auto matLight = static_cast<const Emission*>(
                          shadingInfoNext.material);

                        color +=
                          misWeight * ray.weight * matLight->GetLightColor();
                    }

                    break;
                }

                shadingInfo = shadingInfoNext;
            }
            else
            {
                // IBL
                // TODO: GetEnvironment()を知ってるのおかしい
                if (scene.GetEnvironment().UseEnvImportanceSampling())
                {
                    const float cos =
                      abs(Math::Dot(shadingInfo.normal, ray.dir));
                    const float sin = sqrt(std::max(0.0f, 1.0f - cos * cos));

                    const float pdfBSDF = mat->PDF(prevRay, ray, shadingInfo);
                    float pdfEnv = 0.0f;
                    float misWeight = 0.0;
                    if (ray.dir.x != 0 && ray.dir.y != 0 && sin > 0)
                    {
                        pdfEnv =
                          scene.GetEnvironment().GetImportanceSamplingPDF(
                            ray.dir) /
                          (2.0f * Math::kPi * Math::kPi * sin);

#ifdef BALANCE_HEURISTIC
                        misWeight = pdfBSDF / (pdfEnv + pdfBSDF);
#else
                        misWeight = pdfBSDF * pdfBSDF /
                                    (pdfEnv * pdfEnv + pdfBSDF * pdfBSDF);
#endif
                    }
                    else
                    {
                        misWeight = 1.0f;
                    }

                    const Color3f contribution =
                      misWeight * ray.weight *
                      scene.GetEnvironment().GetColor(ray.dir);
                    ASSERT(contribution.MinElem() >= 0);

                    color += contribution;
                }
                else
                {
                    color +=
                      ray.weight * scene.GetEnvironment().GetColor(ray.dir);
                    ASSERT(color.MinElem() >= 0.0f);
                }

                break;
            }
        }

        ASSERT(color.MinElem() >= 0.0f);
        if (color.MinElem() >= 0.0f)
        {
            pixelColorSum += color;
        }
    }

    Color3f averagedColor = pixelColorSum / static_cast<float>(numSamples);
    targetTex->SetPixel(pixelX, pixelY, averagedColor);
}

Color3f
PathTracing::CalcLightContribution(const Scene& scene,
                                   const ShadingInfo& shadingInfo,
                                   ISampler1D& sampler1D,
                                   ISampler2D& sampler2D,
                                   const Ray& ray)
{
    Color3f lightContribution;

    const float dot = -Math::Dot(ray.dir, shadingInfo.normal);
    const Math::Vector3f p =
      shadingInfo.pos + kEps * std::copysign(1.0f, dot) * shadingInfo.normal;
    ;

    float pdfArea = 1.0f;
    bool sampleEnvMap = false;
    const PointData pointOnLight = SampleLight(
      scene, p, sampler1D.Next(), sampler2D, &pdfArea, &sampleEnvMap);

    if (!sampleEnvMap)
    {
        if (scene.GetLights().empty())
        {
            return Color3f::Zero();
        }

        Ray rayToLight(
          p, (pointOnLight.pos - p).Normalized(), RayTypes::Shadow);

        const auto hitInfoLight = scene.Intersect(rayToLight);
        if (hitInfoLight && hitInfoLight->hitObj->GetMaterial(sampler1D.Next())
                                ->GetMaterialType() == MaterialTypes::Emission)
        {
            const ShadingInfo shadingInfoLight =
              (*hitInfoLight->hitObj).Interpolate(rayToLight, *hitInfoLight);
            const float dist =
              (pointOnLight.pos - shadingInfoLight.pos).SquaredLength();

            const bool isHitToLight = Math::ApproxEq(dist, 0.0f, kEps);
            if (isHitToLight)
            {
                const MaterialBase* const mat = shadingInfo.material;

                const float l2 =
                  (shadingInfoLight.pos - shadingInfo.pos).SquaredLength();

                const float cosP =
                  std::abs(Dot(rayToLight.dir, shadingInfoLight.normal));

                float misWeight = 0.0f;
                if (l2 > 0.0f)
                {
                    const float pdfLight = pdfArea;
                    const float pdfBSDF =
                      mat->PDF(ray, rayToLight, shadingInfo) * cosP / l2;
#ifdef BALANCE_HEURISTIC
                    misWeight = pdfLight / (pdfLight + pdfBSDF);
#else
                    misWeight = pdfLight * pdfLight /
                                (pdfLight * pdfLight + pdfBSDF * pdfBSDF);
#endif
                }

                const auto matEmission =
                  static_cast<const Emission*>(shadingInfoLight.material);
                const Color3f li = matEmission->GetLightColor();
                const Color3f f = mat->BxDF(ray, rayToLight, shadingInfo);
                auto cos =
                  std::abs(Math::Dot(rayToLight.dir, shadingInfo.normal));

                ASSERT(std::isfinite(misWeight) && misWeight >= 0.0f);

                lightContribution += misWeight * ray.weight *
                                     (li * f * cos * cosP / (pdfArea * l2));
            }
        }
    }
    else if (scene.GetEnvironment().UseEnvImportanceSampling())
    {
        float pdfuv = 0.0f;
        Math::Vector3f sampledDir =
          scene.GetEnvironment().ImportanceSampling(sampler2D, &pdfuv);

        const Math::Vector3f rayOrigin =
          shadingInfo.pos +
          kEps * std::copysign(1.0f, dot) * shadingInfo.normal;

        Ray rayToEnv(rayOrigin, sampledDir, RayTypes::Shadow);
        const std::optional<HitInfo> hitInfo = scene.Intersect(rayToEnv);

        // 物体に遮られず、環境マップが見えた場合
        if (hitInfo == std::nullopt)
        {
            const float cos =
              std::abs(Math::Dot(rayToEnv.dir, shadingInfo.normal));
            const float sin = sqrt(std::max(0.0f, 1.0f - cos * cos));
            const MaterialBase* const mat = shadingInfo.material;

            const float pdfBSDF = 2.0f * Math::kPi * Math::kPi * sin *
                                  mat->PDF(ray, rayToEnv, shadingInfo);

#ifdef BALANCE_HEURISTIC
            float misWeight = pdfuv / (pdfuv + pdfBSDF);
#else
            float misWeight =
              pdfuv * pdfuv / (pdfuv * pdfuv + pdfBSDF * pdfBSDF);
#endif

            const Color3f li = scene.GetEnvironment().GetColor(rayToEnv.dir);
            const Color3f f = mat->BxDF(ray, rayToEnv, shadingInfo);

            const Color3f contribution =
              misWeight * ray.weight *
              (li * f * sin * cos * 2.0f * Math::kPi * Math::kPi) / pdfuv;
            ASSERT(contribution.MinElem() >= 0);

            lightContribution += contribution;
        }
    }

    ASSERT(lightContribution.MinElem() >= 0);
    return lightContribution;
}

PointData
PathTracing::SampleLight(const Scene& scene,
                         const Math::Vector3f& shadowRayOrigin,
                         float randomVal,
                         ISampler2D& sampler2D,
                         float* pdfArea,
                         bool* sampleEnvMap)
{
    const auto& lights = scene.GetLights();

    int index = 0;
    if (sampleEnvMap == nullptr)
    {
        index = static_cast<int>(randomVal * lights.size());
        index = std::min(index, static_cast<int>(lights.size() - 1));

        if (sampleEnvMap)
        {
            *sampleEnvMap = false;
        }
    }
    else
    {
        index = static_cast<int>(randomVal * (lights.size() + 1));
        index = std::min(index, static_cast<int>(lights.size()));
        ASSERT(index < lights.size() + 1);

        if (index >= lights.size())
        {
            *sampleEnvMap = true;
            return PointData{};
        }
    }

    float sumArea = 0.0f;

    PointData result;
    for (size_t i = 0; i < lights.size(); i++)
    {
        float pdfAreaEach = 0.0f;
        PointData pointOnSurface;
        lights[i]->SampleSurface(
          shadowRayOrigin, sampler2D, &pointOnSurface, &pdfAreaEach);
        sumArea += 1.0f / pdfAreaEach;

        if (i == index)
        {
            result = pointOnSurface;
        }
    }

    if (pdfArea)
    {
        *pdfArea = 1.0f / sumArea;
    }

    return result;
}

} // namespace Core
} // namespace Petrichor
