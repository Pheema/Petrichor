#include "AOVDenoisingNormal.h"

#include "Core/HitInfo.h"
#include "Core/Material/GGX.h"
#include "Core/Material/Glass.h"
#include "Core/Material/Lambert.h"
#include "Core/Scene.h"
#include <Random/XorShift.h>
// #include <algorithm>
// #include <sstream>

namespace Petrichor
{
namespace Core
{

void
AOVDenoisingNormal::Render(uint32_t pixelX,
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

    Color3f contributionSum;
    for (int spp = 0; spp < numSamples; spp++)
    {
        auto ray = mainCamera->GenerateRay(pixelX,
                                           pixelY,
                                           targetTex->GetWidth(),
                                           targetTex->GetHeight(),
                                           sampler2D);

        contributionSum += CalcPathContribution(ray, accel, scene, sampler2D);
    }

    const Color3f contributionAverage =
      contributionSum / static_cast<float>(numSamples);
    targetTex->SetPixel(pixelX, pixelY, contributionAverage);
}

Color3f
AOVDenoisingNormal::CalcPathContribution(const Ray& cameraRay,
                                         const AccelBase& accel,
                                         const Scene& scene,
                                         ISampler2D& sampler2D)
{
    Ray ray = cameraRay;

    const int numMaxBounces = scene.GetRenderSetting().numMaxBounces;
    for (int bounce = 0; bounce < numMaxBounces; bounce++)
    {
        const auto hitInfo = accel.Intersect(ray, kEps);
        if (!hitInfo)
        {
            return ray.throughput * (-ray.dir);
        }

        const auto shadingInfo = (*hitInfo->hitObj).Interpolate(ray, *hitInfo);
        if (!shadingInfo.material)
        {
            return ray.throughput * shadingInfo.normal;
        }

        switch (shadingInfo.material->GetMaterialType())
        {
        case Core::MaterialTypes::Lambert:
        {
            return ray.throughput * shadingInfo.normal;
        }
        case Core::MaterialTypes::Glass:
        {
            const auto* const glass =
              static_cast<const Core::Glass*>(shadingInfo.material);

            if (glass->GetAlpha(shadingInfo) != 0)
            {
                return ray.throughput * shadingInfo.normal;
            }
            else
            {
                ray = shadingInfo.material->CreateNextRay(
                  ray, shadingInfo, sampler2D);
            }
            break;
        }
        case Core::MaterialTypes::GGX:
        {
            const auto* const ggx =
              static_cast<const Core::GGX*>(shadingInfo.material);

            if (ggx->GetAlpha(shadingInfo) != 0)
            {
                return ray.throughput * shadingInfo.normal;
            }
            else
            {
                ray = shadingInfo.material->CreateNextRay(
                  ray, shadingInfo, sampler2D);
            }
            break;
        }
        default:
        {
            ASSERT(false && "Invalid material type.");
            return Color3f::Zero();
        }
        }
    }

    return Color3f::Zero();
}

} // namespace Core
} // namespace Petrichor
