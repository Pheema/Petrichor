﻿#include "PathTracing.h"

#include "Core/Accel/BVH.h"
#include "Core/Accel/BruteForce.h"
#include "Core/HitInfo.h"
#include "Core/Sampler/MicroJitteredSampler.h"
#include "Core/Sampler/RandomSampler2D.h"
#include "Core/Scene.h"
#include "Core/Texture2D.h"
#include "Core/TileManager.h"
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
    x              = Petrichor::Math::Clamp(x, 0.0f, 1.0f);
    Color3f value  = 2.1f * x * Color3f::One() - Color3f(1.8f, 1.14f, 0.3f);
    Color3f result = Color3f::One() - value * value;
    return Color3f(Petrichor::Math::Clamp(result.x, 0.0f, 1.0f),
                   Petrichor::Math::Clamp(result.y, 0.0f, 1.0f),
                   Petrichor::Math::Clamp(result.z, 0.0f, 1.0f));
}

void
PathTracing::Render(const Scene& scene, Texture2D* targetTex)
{
    const auto mainCamera = scene.GetMainCamera();
    if (mainCamera == nullptr)
    {
        std::cout << "Main camera has not set to the scene." << std::endl;
        return;
    }

    BVH accel;
    /// BruteForce accel;
    accel.Build(scene);

    const auto w = targetTex->GetWidth();
    const auto h = targetTex->GetHeight();

    TileManager tileManager(w, h, 16, 16);

#pragma omp parallel for schedule(dynamic)
    for (int idxTile = 0; idxTile < tileManager.GetNumTiles(); idxTile++)
    {
        // TODO: RNGを宣言。各関数へ参照渡し
        Math::XorShift128 rng(idxTile);
        RandomSampler2D rng2D(idxTile);

        // MicroJitteredSampler rng2DHalton(idxTile);
        // rng2DHalton.Initialize(128);

        Tile tile = tileManager.GetTile();

        const auto pixelPos = tile.GetInitialPixel();
        const uint32_t i0 = pixelPos.first;
        const uint32_t j0 = pixelPos.second;
        for (uint32_t j = j0; j < j0 + tile.GetHeight(); j++)
        {
            for (uint32_t i = i0; i < i0 + tile.GetWidth(); i++)
            {
                Color3f pixelColorSum;
                const uint32_t kSamplesPerPixel = 128;
                for (uint32_t spp = 0; spp < kSamplesPerPixel; spp++)
                {
                    Color3f color;
                    auto ray = mainCamera->PixelToRay(i, j, w, h, rng2D);

                    HitInfo hitInfo;
                    const MaterialBase* mat;
                    if (accel.Intersect(ray, &hitInfo, kEps))
                    {
                        mat = (hitInfo.hitObj)->GetMaterial(rng.next());
                        if (mat->GetMaterialType() == MaterialTypes::Emission)
                        {
                            pixelColorSum +=
                              ray.weight * mat->Radiance(ray, ray, hitInfo);
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

                    // ---- ヒットした場合 ----
                    const uint32_t kMaxBounce = 64;
                    for (int bounce = 0; bounce < kMaxBounce; bounce++)
                    {

                        // ---- ライトをサンプリング ----
                        if (!scene.GetLights().empty())
                        {
                            float pdfArea     = 0.0f;
                            auto p            = hitInfo.pos;
                            auto pointOnLight = SampleLight(
                              scene, p, rng.next(), rng2D, &pdfArea);
                            Ray rayToLight(p,
                                           (pointOnLight.pos - p).Normalized(),
                                           RayTypes::Shadow);

                            HitInfo hitInfoOfLight;
                            if (accel.Intersect(
                                  rayToLight, &hitInfoOfLight, kEps) &&
                                hitInfoOfLight.hitObj->GetMaterial(rng.next())
                                    ->GetMaterialType() ==
                                  MaterialTypes::Emission)
                            {
                                ASSERT(hitInfoOfLight.distance > 0.0f);

                                float eps =
                                  (pointOnLight.pos - hitInfoOfLight.pos)
                                    .SquaredLength();
                                if (Math::ApproxEq(eps, 0.0f, kEps))
                                {
                                    float pdfDirLight = 0.0f;
                                    float pdfDirBSDF  = 0.0f;
                                    float misWeight   = 1.0f;

                                    const auto& matEmission =
                                      (hitInfoOfLight.hitObj)
                                        ->GetMaterial(0.0f);
                                    auto li = matEmission->Radiance(
                                      rayToLight, rayToLight, hitInfoOfLight);
                                    auto f =
                                      mat->BRDF(ray, rayToLight, hitInfo);
                                    auto cos = std::abs(Math::Dot(
                                      rayToLight.dir, hitInfo.normal));

                                    float l2 =
                                      (hitInfoOfLight.pos - hitInfo.pos)
                                        .SquaredLength();

                                    float cosP = std::abs(Dot(
                                      -rayToLight.dir, hitInfoOfLight.normal));
                                    if (cosP >= 1.0e-4)
                                    {
                                        pdfDirLight = (l2 * pdfArea) / cosP;
                                        pdfDirBSDF =
                                          mat->PDF(ray, rayToLight, hitInfo);
#ifdef BALANCE_HEURISTIC
                                        misWeight = pdfDirLight /
                                                    (pdfDirLight + pdfDirBSDF);
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

                                    ASSERT(std::isfinite(misWeight) &&
                                           misWeight >= 0.0f);
                                    color += misWeight * ray.weight *
                                             (li * f * cos / pdfDirLight);
                                    ASSERT(color.MinElem() >= 0.0f);
                                }
                            }
                        }

                        // 次のレイを生成
                        float pdfDir = 0.0f;
                        mat = (hitInfo.hitObj)->GetMaterial(rng.next());
                        ASSERT(mat->GetMaterialType() !=
                               MaterialTypes::Emission);
                        ASSERT(std::isfinite(ray.dir.x));
                        ray = mat->CreateNextRay(ray, hitInfo, rng2D, &pdfDir);
                        ASSERT(std::isfinite(ray.dir.x));

                        // MIS
                        hitInfo.Clear();
                        if (accel.Intersect(ray, &hitInfo, kEps))
                        {
                            mat = (hitInfo.hitObj)->GetMaterial(rng.next());
                            if (mat->GetMaterialType() ==
                                MaterialTypes::Emission)
                            {
                                float pdfDirLight = 0.0f;
                                float pdfDirBSDF  = 0.0f;
                                float misWeight   = 1.0f;

                                float l2 =
                                  (hitInfo.pos - ray.o).SquaredLength();
                                float cos =
                                  abs(Math::Dot(-ray.dir, hitInfo.normal));

                                // TODO:
                                // 下の関数が確率密度関数を取得しているだけなのに無駄
                                float pdfAreaLight;
                                PointData pointData;
                                (hitInfo.hitObj)
                                  ->SampleSurface(
                                    ray.o, rng2D, &pointData, &pdfAreaLight);

                                pdfDirBSDF  = pdfDir;
                                pdfDirLight = l2 / cos * pdfAreaLight;
#ifdef BALANCE_HEURISTIC
                                misWeight =
                                  pdfDirBSDF / (pdfDirLight + pdfDirBSDF);
#else
                                misWeight = pdfDirBSDF * pdfDirBSDF /
                                            (pdfDirLight * pdfDirLight +
                                             pdfDirBSDF * pdfDirBSDF);
#endif
                                ASSERT(std::isfinite(misWeight) &&
                                       misWeight >= 0.0f);
                                color += misWeight * ray.weight *
                                         mat->Radiance(ray, ray, hitInfo);
                                ASSERT(color.MinElem() >= 0.0f);
                                break;
                            }
                        }
                        else
                        {
                            // IBL
                            // TODO: GetEnvironment()を知ってるのおかしい
                            color += ray.weight *
                                     scene.GetEnvironment().GetColor(ray.dir);
                            ASSERT(color.MinElem() >= 0.0f);
                            break;
                        }

                        // 最大反射回数未満の場合はロシアンルーレットを行わない
                        if (ray.bounce < kMaxBounce)
                        {
                            continue;
                        }

                        // TODO: これあるとノイズ乗りやすい
                        // モンテカルロ

                        float reccurenceProb = ray.weight.MaxElem() * 1e3f;
                        if (rng.next() < reccurenceProb)
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
                targetTex->SetPixel(
                  i, j, pixelColorSum / static_cast<float>(kSamplesPerPixel));
            }
        }

        std::stringstream ss;
        ss << "[PT: Rendering] " << (idxTile + 1) << "/"
           << tileManager.GetNumTiles() << std::endl;
        std::cout << ss.str();
    }
}

// #pragma optimize("", off)
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
