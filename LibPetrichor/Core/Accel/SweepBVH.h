﻿#pragma once

#include "AccelBase.h"
#include "BVHNode.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/HitInfo.h"
#include <optional>

namespace Petrichor
{
namespace Core
{

class Scene;
struct HitInfo;
struct Ray;

class SweepBVH : public AccelBase
{
public:
    SweepBVH() = default;

    virtual void
    Build(const Scene& scene) override;

    virtual std::optional<HitInfo>
    Intersect(const Ray& ray,
              float distMin = 0.0f,
              float distMax = kInfinity) const override;

private:
    std::vector<BVHNode> m_bvhNodes;
    std::vector<Bounds> m_bounds;
    std::vector<uint32_t> m_entityIDs;
    const Scene* m_scene = nullptr;
};

} // namespace Core
} // namespace Petrichor
