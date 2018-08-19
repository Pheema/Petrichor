#include "SimplePathTracing.h"

#include "Core/HitInfo.h"
#include "Core/Material/Emission.h"
#include "Core/Sampler/MicroJitteredSampler.h"
#include "Core/Scene.h"
#include <Random/XorShift.h>
#include <algorithm>
#include <sstream>

namespace Petrichor
{
namespace Core
{

void
SimplePathTracing::Render(uint32_t pixelX,
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

    const uint32_t kNumSamples = scene.GetSceneSettings().numSamplesPerPixel;
    const uint32_t kMaxNumBounces = scene.GetSceneSettings().numMaxBouces;

    Color3f pixelColorSum;
    for (uint32_t spp = 0; spp < kNumSamples; spp++)
    {
        Color3f color;
        auto ray = mainCamera->PixelToRay(pixelX,
                                          pixelY,
                                          targetTex->GetWidth(),
                                          targetTex->GetHeight(),
                                          sampler2D);

        bool isOverBounced = false;
        for (uint32_t bounce = 0;; bounce++)
        {
            const auto hitInfo = scene.Intersect(ray, kEps);

            // ---- ヒットしなかった場合 ----
            if (hitInfo == std::nullopt)
            {
                // IBL
                color += ray.weight * scene.GetEnvironment().GetColor(ray.dir);
                break;
            }

            // ---- ヒットした場合 ----
            const MaterialBase* mat =
              (hitInfo->hitObj)->GetMaterial(sampler1D.Next());
            if (mat->GetMaterialType() == MaterialTypes::Emission)
            {
                auto matEmission = static_cast<const Emission*>(mat);
                color += ray.weight * matEmission->GetLightColor();
                break;
            }

            // 次のレイを生成
            mat = (hitInfo->hitObj)->GetMaterial(sampler1D.Next());
            ASSERT(mat->GetMaterialType() != MaterialTypes::Emission);
            ASSERT(std::isfinite(ray.dir.x));

            const auto shadingInfo =
              (*hitInfo->hitObj).Interpolate(ray, hitInfo.value());
            ray = mat->CreateNextRay(ray, shadingInfo, sampler2D);

            // 最大反射回数未満の場合はロシアンルーレットを行わない
            // TODO: あとでロシアンルーレット方式に
            if (ray.bounce >= kMaxNumBounces)
            {
                isOverBounced = true;
                break;
            }
        }

        ASSERT(color.MinElem() >= 0.0f);
        if (!isOverBounced)
        {
            pixelColorSum += color;
        }
    }

    const Color3f averagedColor =
      pixelColorSum / static_cast<float>(kNumSamples);
    targetTex->SetPixel(pixelX, pixelY, averagedColor);
}

} // namespace Core
} // namespace Petrichor
