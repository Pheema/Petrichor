#pragma once

#include "Core/Constants.h"
#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

class GeometryBase;
class MaterialBase;

struct HitInfo
{
    float distance = kInfinity; // 反射点から衝突点までの距離
    const GeometryBase* hitObj = nullptr; // 衝突したジオメトリへのポインタ
};

struct ShadingInfo
{
    Math::Vector3f normal;
    Math::Vector3f pos;
    Math::Vector3f uv;
    const MaterialBase* material = nullptr;
};

} // namespace Core
} // namespace Petrichor
