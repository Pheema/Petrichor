#pragma once

#include "Core/Constants.h"
#include "Core/HitInfo.h"
#include <limits>
#include <optional>

namespace Petrichor
{
namespace Core
{

struct Ray;
class Scene;

enum class AccelType
{
    BruteForce, // 構造を使用しない
    BVH         // BVH
};

//!< #TODO: 仮
struct PrecalcedData
{
    Math::Vector3f invRayDir{};
    int8_t sign[3] = {};
};

class AccelBase
{
public:
    //! 構築
    virtual void
    Build(const Scene& scene) = 0;

    //! 交差判定
    virtual std::optional<HitInfo>
    Intersect(const Ray& ray,
              const Scene& scene,
              float distMin,
              float distMax) const = 0;

    //! 交差判定(デフォルト引数版)
    std::optional<Petrichor::Core::HitInfo>
    Intersect(const Ray& ray, const Scene& scene) const
    {
        return Intersect(ray, scene, 0.0f, std::numeric_limits<float>::max());
    }

    //! 交差判定(デフォルト引数版)
    std::optional<HitInfo>
    Intersect(const Ray& ray, const Scene& scene, float distMin) const
    {
        return Intersect(ray, scene, distMin, kInfinity);
    }
};

} // namespace Core
} // namespace Petrichor
