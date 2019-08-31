#include "AOVDenoisingNormal.h"

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

    Color3f sumPixelColor;
    for (int spp = 0; spp < numSamples; spp++)
    {
        auto ray = mainCamera->GenerateRay(pixelX,
                                           pixelY,
                                           targetTex->GetWidth(),
                                           targetTex->GetHeight(),
                                           sampler2D);

        const auto hitInfo = accel.Intersect(ray, kEps);
        if (hitInfo)
        {
            const auto shadingInfo =
              (*hitInfo->hitObj).Interpolate(ray, *hitInfo);
            sumPixelColor += shadingInfo.normal;
        }
        else
        {
            sumPixelColor += -ray.dir;
        }
    }

    const Color3f avaragePixelColor =
      sumPixelColor / static_cast<float>(numSamples);
    targetTex->SetPixel(pixelX, pixelY, avaragePixelColor);
}

} // namespace Core
} // namespace Petrichor
