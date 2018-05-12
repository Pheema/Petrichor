#pragma once

#include "Core/Geometry/GeometryBase.h"

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
    Render(const Scene& scene, Texture2D* targetTex);

private:
    // ランダムにライト上をサンプリング
    PointData
    SampleLight(const Scene& scene,
                const Math::Vector3f& p,
                float randomVal,
                ISampler2D& sampler2D,
                float* pdfArea);
};
}
}
