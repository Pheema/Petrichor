#pragma once

#include "Core/Geometry/GeometryBase.h"
#include "Core/Sampler/ISampler1D.h"
#include "Core/Sampler/ISampler2D.h"

namespace Petrichor
{
namespace Core
{

struct HitInfo;
class ISampler2D;
struct Ray;
class Scene;
class Texture2D;

class PathTracing
{
public:
    PathTracing() = default;

    void
    Render(uint32_t pixelX,
           uint32_t pixelY,
           const Scene& scene,
           Texture2D* targetTex,
           ISampler1D& sampler1D,
           ISampler2D& sampler2D);

private:
    // ランダムにライト上をサンプリング
    PointData
    SampleLight(const Scene& scene,
                const Math::Vector3f& p,
                float randomVal,
                ISampler2D& sampler2D,
                float* pdfArea);
};
} // namespace Core
} // namespace Petrichor
