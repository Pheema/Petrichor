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
                          const AccelBase& accel,
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

    const int numSamples = scene.GetRenderSetting().numSamplesPerPixel;
    const int maxNumBounces = scene.GetRenderSetting().numMaxBounces;

    Color3f sumContribution;
    for (int spp = 0; spp < numSamples; spp++)
    {
        auto ray = mainCamera->GenerateRay(pixelX,
                                           pixelY,
                                           targetTex->GetWidth(),
                                           targetTex->GetHeight(),
                                           sampler2D);

        Color3f contribution;
        for (int bounce = 0;; bounce++)
        {
            const auto hitInfo = accel.Intersect(ray, kEps);

            // ヒットしなかった場合
            if (!hitInfo)
            {
                // IBL
                contribution +=
                  ray.throughput * scene.GetEnvironment().GetColor(ray.dir);
                break;
            }

            // ---- ヒットした場合 ----
            const MaterialBase* mat =
              (hitInfo->hitObj)->GetMaterial(sampler1D.Next());
            if (mat->GetMaterialType() == MaterialTypes::Emission)
            {
                auto matEmission = static_cast<const Emission*>(mat);
                contribution += ray.throughput * matEmission->GetLightColor();
                break;
            }

            // 次のレイを生成
            mat = (hitInfo->hitObj)->GetMaterial(sampler1D.Next());
            ASSERT(mat->GetMaterialType() != MaterialTypes::Emission);
            ASSERT(std::isfinite(ray.dir.x));

            const auto shadingInfo =
              (*hitInfo->hitObj).Interpolate(ray, hitInfo.value());
            ray = mat->CreateNextRay(ray, shadingInfo, sampler2D);

            // 最大反射回数以上でロシアンルーレット
            if (ray.bounce > maxNumBounces)
            {
                // #TODO: 大雑把なので条件を考える
                ray.prob *= 0.9f;
                ray.prob = std::max(0.1f, ray.prob);
                if (sampler1D.Next() < ray.prob)
                {
                    ray.throughput /= ray.prob;
                }
                else
                {
                    break;
                }
            }
        }

        ASSERT(contribution.MinElem() >= 0.0f);
        sumContribution += contribution;
    }

    const Color3f averagedContribution =
      sumContribution / static_cast<float>(numSamples);
    targetTex->SetPixel(pixelX, pixelY, averagedContribution);
}

} // namespace Core
} // namespace Petrichor
