#pragma once

#include "Core/Accel/AccelBase.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Sampler/ISampler1D.h"
#include "Core/Sampler/ISampler2D.h"
#include "Core/Texture2D.h"

namespace Petrichor
{
namespace Core
{

class AOVDenoisingNormal
{
public:
    void
    Render(uint32_t pixelX,
           uint32_t pixelY,
           const Scene& scene,
           const AccelBase& accel,
           Texture2D* targetTex,
           ISampler1D& sampler1D,
           ISampler2D& sampler2D);

private:
    Color3f
    CalcPathContribution(const Ray& cameraRay,
                         const AccelBase& accel,
                         const Scene& scene,
                         ISampler2D& sampler2D);
};

} // namespace Core
} // namespace Petrichor
