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

#define BALANCE_HEURISTIC

namespace Petrichor
{
namespace Core
{

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

        auto hitInfo = scene.Intersect(ray, kEps);
        if (hitInfo == std::nullopt)
        {
            // IBL
            pixelColorSum +=
              ray.weight * scene.GetEnvironment().GetColor(ray.dir);
            continue;
        }

        auto shadingInfo = (*hitInfo->hitObj).Interpolate(ray, hitInfo.value());

        // ---- 光源に直接ヒットした場合 ----
        const MaterialBase* mat =
          (hitInfo->hitObj)->GetMaterial(sampler1D.Next());
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
              scene, shadingInfo, sampler1D, sampler2D, ray, hitInfo, mat);

            // 次のレイを生成
            float pdfDir = 0.0f;
            mat = (hitInfo->hitObj)->GetMaterial(sampler1D.Next());
            ASSERT(mat->GetMaterialType() != MaterialTypes::Emission);
            ASSERT(std::isfinite(ray.dir.x));
            ray = mat->CreateNextRay(ray, shadingInfo, sampler2D, &pdfDir);

            // MIS
            if (const auto hitInfoNext = scene.Intersect(ray, kEps);
                hitInfoNext)
            {
                hitInfo = hitInfoNext;

                const ShadingInfo shadingInfoNext =
                  hitInfoNext->hitObj->Interpolate(ray, *hitInfoNext);

                shadingInfo = shadingInfoNext;

                if (shadingInfoNext.material->GetMaterialType() ==
                    MaterialTypes::Emission)
                {
                    float pdfDirLight = 0.0f;
                    float pdfDirBSDF = 0.0f;
                    float misWeight = 1.0f;

                    const float l2 =
                      (shadingInfoNext.pos - ray.o).SquaredLength();
                    const float cos =
                      abs(Math::Dot(-ray.dir, shadingInfoNext.normal));

                    // TODO:
                    // 下の関数が確率密度関数を取得しているだけなのに無駄
                    float pdfAreaLight = 0.0f;
                    PointData pointData;
                    (hitInfoNext->hitObj)
                      ->SampleSurface(
                        ray.o, sampler2D, &pointData, &pdfAreaLight);

                    pdfDirBSDF = pdfDir;
                    pdfDirLight = l2 / cos * pdfAreaLight;
#ifdef BALANCE_HEURISTIC
                    misWeight = pdfDirBSDF / (pdfDirLight + pdfDirBSDF);
#else
                    misWeight =
                      pdfDirBSDF * pdfDirBSDF /
                      (pdfDirLight * pdfDirLight + pdfDirBSDF * pdfDirBSDF);
#endif
                    ASSERT(std::isfinite(misWeight) && misWeight >= 0.0f);

                    {
                        auto matLight = static_cast<const Emission*>(
                          shadingInfoNext.material);
                        color +=
                          misWeight * ray.weight * matLight->GetLightColor();
                    }

                    ASSERT(color.MinElem() >= 0.0f);
                    break;
                }
            }
            else
            {
                // IBL
                // TODO: GetEnvironment()を知ってるのおかしい
                color += ray.weight * scene.GetEnvironment().GetColor(ray.dir);
                ASSERT(color.MinElem() >= 0.0f);
                break;
            }

            //// モンテカルロ
            // if (ray.bounce >= maxNumBounces)
            //{
            //    float reccurenceProb = ray.weight.MaxElem() * 1e3f;
            //    if (sampler1D.Next() < reccurenceProb)
            //    {
            //        ray.weight /= reccurenceProb;
            //    }
            //    else
            //    {
            //        break;
            //    }
            //}
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

Petrichor::Color3f
PathTracing::CalcLightContribution(const Scene& scene,
                                   const ShadingInfo& shadingInfo,
                                   ISampler1D& sampler1D,
                                   ISampler2D& sampler2D,
                                   const Ray& ray,
                                   const std::optional<HitInfo>& hitInfo,
                                   const MaterialBase* mat)
{
    if (scene.GetLights().empty())
    {
        return Color3f::Zero();
    }

    Color3f lightContribution;

    float pdfArea = 0.0f;
    const Math::Vector3f p = shadingInfo.pos;
    const PointData pointOnLight =
      SampleLight(scene, p, sampler1D.Next(), sampler2D, &pdfArea);

    Ray rayToLight(p, (pointOnLight.pos - p).Normalized(), RayTypes::Shadow);

    const auto hitInfoLight = scene.Intersect(rayToLight, kEps);
    if (hitInfoLight && hitInfoLight->hitObj->GetMaterial(sampler1D.Next())
                            ->GetMaterialType() == MaterialTypes::Emission)
    {
        const ShadingInfo shadingInfoLight =
          (*hitInfoLight->hitObj).Interpolate(rayToLight, *hitInfoLight);
        const float eps =
          (pointOnLight.pos - shadingInfoLight.pos).SquaredLength();

        const bool isHitToLight = Math::ApproxEq(eps, 0.0f, kEps);
        if (isHitToLight)
        {
            float pdfDirLight = 1.0f;
            float pdfDirBSDF = 0.0f;
            float misWeight = 0.0f;

            const float l2 =
              (shadingInfoLight.pos - shadingInfo.pos).SquaredLength();

            const float cosP =
              std::abs(Dot(-rayToLight.dir, shadingInfoLight.normal));
            if (cosP > 0.0f)
            {
                pdfDirLight = (l2 * pdfArea) / cosP;
                pdfDirBSDF = mat->PDF(ray, rayToLight, shadingInfo);
#ifdef BALANCE_HEURISTIC
                misWeight = pdfDirLight / (pdfDirLight + pdfDirBSDF);
#else
                misWeight =
                  pdfDirLight * pdfDirLight /
                  (pdfDirLight * pdfDirLight + pdfDirBSDF * pdfDirBSDF);
#endif
            }

            const auto matEmission =
              static_cast<const Emission*>(shadingInfoLight.material);
            const Color3f li = matEmission->GetLightColor();
            const Color3f f = mat->BxDF(ray, rayToLight, shadingInfo);
            auto cos = std::abs(Math::Dot(rayToLight.dir, shadingInfo.normal));

            ASSERT(std::isfinite(misWeight) && misWeight >= 0.0f);
            lightContribution =
              misWeight * ray.weight * (li * f * cos / pdfDirLight);
            ASSERT(lightContribution.MinElem() >= 0.0f);
        }
    }
    return lightContribution;
}

PointData
PathTracing::SampleLight(const Scene& scene,
                         const Math::Vector3f& p,
                         float randomVal,
                         ISampler2D& sampler2D,
                         float* pdfArea)
{
    const auto& lights = scene.GetLights();

    auto index = static_cast<size_t>(randomVal * lights.size());
    if (index >= lights.size())
    {
        std::cout << index << std::endl;
        index = lights.size() - 1;
    }
    ASSERT(index < lights.size());

    float sumArea = 0.0f;

    PointData result;
    for (size_t i = 0; i < lights.size(); i++)
    {
        float pdfAreaEach = 0.0f;
        PointData pointOnSurface;
        lights[i]->SampleSurface(p, sampler2D, &pointOnSurface, &pdfAreaEach);
        sumArea += 1.0f / pdfAreaEach;

        if (i == index)
        {
            result = pointOnSurface;
        }
    }

    *pdfArea = 1.0f / sumArea;

    return result;
}

} // namespace Core
} // namespace Petrichor
