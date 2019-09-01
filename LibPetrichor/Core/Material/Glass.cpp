#include "Glass.h"

#include "Core/Ray.h"
#include "Core/Sampler/ISampler2D.h"

namespace Petrichor
{
namespace Core
{

Color3f
Glass::BxDF(const Ray& rayIn,
            const Ray& rayOut,
            const ShadingInfo& shadingInfo) const
{
    // #TODO: roughnessを追加したらここも変更
    return Color3f::Zero();
}

float
Glass::PDF(const Ray& rayIn,
           const Ray& rayOut,
           const ShadingInfo& shadingInfo) const
{
    // #TODO: roughnessを追加したらここも変更
    return 0.0f;
}

Ray
Glass::CreateNextRay(const Ray& rayIn,
                     const ShadingInfo& shadingInfo,
                     ISampler2D& sampler2D) const
{
    const float hitSign = -Math::Dot(rayIn.dir, shadingInfo.normal);
    const Math::Vector3f normal =
      shadingInfo.normal * std::copysign(1.0f, hitSign);

    ASSERT(0 < rayIn.ior);
    const float relativeIOR = m_ior / rayIn.ior;

    Ray rayOut;

    const auto refractDir =
      rayIn.dir.Refracted(shadingInfo.normal, relativeIOR);

    if (!refractDir)
    {
        // 全反射
        rayOut.o = shadingInfo.pos + kEps * normal;
        rayOut.dir = rayIn.dir.Reflected(normal).Normalized();
        rayOut.throughput = Math::Vector3f::Zero(); //; rayIn.throughput;
        rayOut.rayType = RayTypes::Glossy;
        rayOut.bounce = rayOut.bounce + 1;
        rayOut.prob = rayIn.prob;
        rayOut.ior = rayIn.ior;
        return rayOut;
    }

    rayOut.dir = (*refractDir).Normalized();

    const float normalDotOutDir = Math::Dot(shadingInfo.normal, rayOut.dir);
    const float refrectance = [&] {
        const float f0 =
          Math::Pow<2>((1.0f - relativeIOR) / (1.0f + relativeIOR));

        return f0 +
               (1.0f - f0) * Math::Pow<5>(1.0f - std::abs(normalDotOutDir));
    }();

    if (std::get<0>(sampler2D.Next()) <= refrectance)
    {
        // 反射
        rayOut.o = shadingInfo.pos + kEps * normal;
        rayOut.throughput = rayIn.throughput;
        rayOut.rayType = RayTypes::Glossy;
        rayOut.bounce = rayIn.bounce + 1;
        rayOut.prob = rayIn.prob;
        rayOut.ior = rayIn.ior;
        return rayOut;
    }
    else
    {
        const bool isEnterMedium = (normalDotOutDir < 0);

        // 透過
        rayOut.o = shadingInfo.pos - kEps * normal;
        rayOut.throughput = rayIn.throughput;
        rayOut.rayType = RayTypes::Glossy;
        rayOut.bounce = rayIn.bounce + 1;
        rayOut.prob = rayIn.prob;
        rayOut.ior =
          isEnterMedium ? rayOut.ior * relativeIOR : rayOut.ior / relativeIOR;
        return rayOut;
    }
}

} // namespace Core
} // namespace Petrichor
