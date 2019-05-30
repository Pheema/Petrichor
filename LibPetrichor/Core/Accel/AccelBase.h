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

class AccelBase
{
public:
    //! 構築
    virtual void
    Build(const Scene& scene) = 0;

    //! 交差判定
    virtual std::optional<HitInfo>
    Intersect(const Ray& ray, float distMin, float distMax) const = 0;

    //! 交差判定(デフォルト引数版)
    std::optional<HitInfo>
    Intersect(const Ray& ray) const
    {
        return Intersect(ray, 0.0f, kInfinity);
    }

    //! 交差判定(デフォルト引数版)
    std::optional<HitInfo>
    Intersect(const Ray& ray, float distMin) const
    {
        return Intersect(ray, distMin, kInfinity);
    }
};

} // namespace Core
} // namespace Petrichor
