#include "PathTracing.h"

#include "Core/Accel/BVH.h"
#include "Core/Accel/BruteForce.h"
#include "Core/HitInfo.h"
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

// TODO:
Color3f
SimplifiedSpectrum(float x)
{
    x = Petrichor::Math::Clamp(x, 0.0f, 1.0f);

    Color3f value  = 2.1f * x * Color3f::One() - Color3f(1.8f, 1.14f, 0.3f);
    Color3f result = Color3f::One() - value * value;
    return Color3f(Petrichor::Math::Clamp(result.x, 0.0f, 1.0f),
                   Petrichor::Math::Clamp(result.y, 0.0f, 1.0f),
                   Petrichor::Math::Clamp(result.z, 0.0f, 1.0f));
}

void
PathTracing::Render(uint32_t pixelX,
                    uint32_t pixelY,
                    const Scene& scene,
                    Texture2D* const targetTex,
                    ISampler1D& sampler1D,
                    ISampler2D& sampler2D)
{
    const auto* const mainCamera = scene.GetMainCamera();
    if (mainCamera == nullptr)
    {
        std::cerr << "[Error] Main camera is not found." << std::endl;
        return;
    }

    constexpr uint32_t numSamples = 64;

    Color3f pixelColorSum;
    for (uint32_t spp = 0; spp < numSamples; spp++)
    {
        Color3f color;
        auto ray = mainCamera->PixelToRay(pixelX,
                                          pixelY,
                                          targetTex->GetWidth(),
                                          targetTex->GetHeight(),
                                          sampler2D);

        HitInfo hitInfo;
        const MaterialBase* mat;
        if (scene.Intersect(ray, &hitInfo, kEps))
        {
            mat = (hitInfo.hitObj)->GetMaterial(sampler1D.Next());
            if (mat->GetMaterialType() == MaterialTypes::Emission)
            {
                pixelColorSum += ray.weight * mat->Radiance(ray, ray, hitInfo);
                continue;
            }
        }
        else
        {
            // IBL
            pixelColorSum +=
              ray.weight * scene.GetEnvironment().GetColor(ray.dir);
            continue;
        }

        constexpr uint32_t maxNumBounces = 64;

        // ---- ヒットした場合 ----
        for (uint32_t bounce = 0; bounce < maxNumBounces; bounce++)
        {

            // ---- ライトをサンプリング ----
            if (!scene.GetLights().empty() && false)
            {
                float pdfArea = 0.0f;
                auto p        = hitInfo.pos;
                auto pointOnLight =
                  SampleLight(scene, p, sampler1D.Next(), sampler2D, &pdfArea);
                Ray rayToLight(
                  p, (pointOnLight.pos - p).Normalized(), RayTypes::Shadow);

                HitInfo hitInfoOfLight;
                if (scene.Intersect(rayToLight, &hitInfoOfLight, kEps) &&
                    hitInfoOfLight.hitObj->GetMaterial(sampler1D.Next())
                        ->GetMaterialType() == MaterialTypes::Emission)
                {
                    ASSERT(hitInfoOfLight.distance > 0.0f);

                    float eps =
                      (pointOnLight.pos - hitInfoOfLight.pos).SquaredLength();
                    if (Math::ApproxEq(eps, 0.0f, kEps))
                    {
                        float pdfDirLight = 0.0f;
                        float pdfDirBSDF  = 0.0f;
                        float misWeight   = 1.0f;

                        const auto& matEmission =
                          (hitInfoOfLight.hitObj)->GetMaterial(0.0f);
                        auto li = matEmission->Radiance(
                          rayToLight, rayToLight, hitInfoOfLight);
                        auto f = mat->BRDF(ray, rayToLight, hitInfo);
                        auto cos =
                          std::abs(Math::Dot(rayToLight.dir, hitInfo.normal));

                        float l2 =
                          (hitInfoOfLight.pos - hitInfo.pos).SquaredLength();

                        float cosP =
                          std::abs(Dot(-rayToLight.dir, hitInfoOfLight.normal));
                        if (cosP >= 1.0e-4)
                        {
                            pdfDirLight = (l2 * pdfArea) / cosP;
                            pdfDirBSDF  = mat->PDF(ray, rayToLight, hitInfo);
#ifdef BALANCE_HEURISTIC
                            misWeight =
                              pdfDirLight / (pdfDirLight + pdfDirBSDF);
#else
                            misWeight = pdfDirLight * pdfDirLight /
                                        (pdfDirLight * pdfDirLight +
                                         pdfDirBSDF * pdfDirBSDF);
#endif
                        }
                        else
                        {
                            pdfDirLight = 1.0f;
                            misWeight   = 0.0f;
                        }

                        ASSERT(std::isfinite(misWeight) && misWeight >= 0.0f);
                        color +=
                          misWeight * ray.weight * (li * f * cos / pdfDirLight);
                        ASSERT(color.MinElem() >= 0.0f);
                    }
                }
            }

            // 次のレイを生成
            float pdfDir = 0.0f;
            mat          = (hitInfo.hitObj)->GetMaterial(sampler1D.Next());
            ASSERT(mat->GetMaterialType() != MaterialTypes::Emission);
            ASSERT(std::isfinite(ray.dir.x));
            ray = mat->CreateNextRay(ray, hitInfo, sampler2D, &pdfDir);
            ASSERT(std::isfinite(ray.dir.x));

            // MIS
            hitInfo.Clear();
            if (scene.Intersect(ray, &hitInfo, kEps))
            {
                mat = (hitInfo.hitObj)->GetMaterial(sampler1D.Next());
                if (mat->GetMaterialType() == MaterialTypes::Emission)
                {
                    float pdfDirLight = 0.0f;
                    float pdfDirBSDF  = 0.0f;
                    float misWeight   = 1.0f;

                    float l2  = (hitInfo.pos - ray.o).SquaredLength();
                    float cos = abs(Math::Dot(-ray.dir, hitInfo.normal));

                    // TODO:
                    // 下の関数が確率密度関数を取得しているだけなのに無駄
                    float pdfAreaLight;
                    PointData pointData;
                    (hitInfo.hitObj)
                      ->SampleSurface(
                        ray.o, sampler2D, &pointData, &pdfAreaLight);

                    pdfDirBSDF  = pdfDir;
                    pdfDirLight = l2 / cos * pdfAreaLight;
#ifdef BALANCE_HEURISTIC
                    misWeight = pdfDirBSDF / (pdfDirLight + pdfDirBSDF);
                    misWeight = 1.0f;
#else
                    misWeight =
                      pdfDirBSDF * pdfDirBSDF /
                      (pdfDirLight * pdfDirLight + pdfDirBSDF * pdfDirBSDF);
#endif
                    ASSERT(std::isfinite(misWeight) && misWeight >= 0.0f);
                    color +=
                      misWeight * ray.weight * mat->Radiance(ray, ray, hitInfo);
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

            // 最大反射回数未満の場合はロシアンルーレットを行わない
            // #TODO: 反射回数をセッティングから読み込む
            if (ray.bounce < numSamples)
            {
                continue;
            }

            // TODO: これあるとノイズ乗りやすい
            // モンテカルロ

            float reccurenceProb = ray.weight.MaxElem() * 1e3f;
            if (sampler1D.Next() < reccurenceProb)
            {
                ray.weight /= reccurenceProb;
            }
            else
            {
                // 打ち切り
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

PointData
PathTracing::SampleLight(const Scene& scene,
                         const Math::Vector3f& p,
                         float randomVal,
                         ISampler2D& sampler2D,
                         float* pdfArea)
{
    const auto& lights = scene.GetLights();

    size_t index = static_cast<size_t>(randomVal * lights.size());
    if (index >= lights.size())
    {
        std::cout << index << std::endl;
        index = lights.size() - 1;
    }
    ASSERT(index < lights.size());

    float sumArea = 0.0f;
    float area    = 0.0f;

    PointData result;
    for (size_t i = 0; i < lights.size(); i++)
    {
        float pdfAreaEach = 0.0f;
        PointData pointOnSurface;
        lights[i]->SampleSurface(p, sampler2D, &pointOnSurface, &pdfAreaEach);
        sumArea += 1.0f / pdfAreaEach;

        if (i == index)
        {
            area   = 1.0f / pdfAreaEach;
            result = pointOnSurface;
        }
    }

    *pdfArea = 1.0f / sumArea;

    return result;
}

} // namespace Core
} // namespace Petrichor
