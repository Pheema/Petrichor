#pragma once

#include "Core/Color3f.h"

namespace Petrichor
{
namespace Core
{

enum class RayTypes
{
    Camera,      // カメラから出射したレイ
    Shadow,      // 光源へのレイ
    Diffuse,     // 拡散面で反射したレイ
    Translucent, // 半透明物体を透過したレイ
    Glossy       // 鏡面で反射したレイ
};

struct Ray
{

    constexpr Ray() = default;

    Ray(const Math::Vector3f& o,
        const Math::Vector3f& dir,
        RayTypes rayType = RayTypes::Camera,
        Color3f weight = Color3f::One(),
        uint32_t bounce = 0,
        float prob = 1.0f,
        float ior = 1.0f)
      : o(o)
      , dir(dir)
      , rayType(rayType)
      , weight(weight)
      , bounce(bounce)
      , prob(prob)
      , ior(ior)
    {
        IS_NORMALIZED(dir);
    }

    Math::Vector3f o;                    // レイの原点
    Math::Vector3f dir;                  // レイの単位方向ベクトル
    RayTypes rayType = RayTypes::Camera; // レイの種類
    Color3f weight = Color3f::One(); // ピクセルの輝度に対する寄与
    uint32_t bounce = 0;             // レイが今まで何回反射したか
    float prob = 1.0f; // 反射確率（ロシアンルーレット打ち切り用）
    float ior = 1.0f; // レイが進行している媒質の絶対屈折率
};

} // namespace Core
} // namespace Petrichor
