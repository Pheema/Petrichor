#pragma once

#include "Core/Color3f.h"

namespace Petrichor
{
namespace Core
{

enum class RayTypes : uint8_t
{
    Camera,      // カメラから出射したレイ
    Shadow,      // 光源へのレイ
    Diffuse,     // 拡散面で反射したレイ
    Translucent, // 半透明物体を透過したレイ
    Glossy,      // 鏡面で反射したレイ
    Refract,     //!< 屈折したレイ
};

struct Ray
{
    Ray() = default;

    Ray(const Math::Vector3f& o,
        const Math::Vector3f& dir,
        RayTypes rayType = RayTypes::Camera,
        Color3f throughput = Color3f::One(),
        int bounce = 0,
        float prob = 1.0f,
        float ior = 1.0f)
      : o(o)
      , dir(dir)
      , rayType(rayType)
      , throughput(throughput)
      , bounce(bounce)
      , prob(prob)
      , ior(ior)
    {
        IS_NORMALIZED(dir);
    }

    Math::Vector3f o;                    // レイの原点
    Math::Vector3f dir;                  // レイの単位方向ベクトル
    Color3f throughput = Color3f::One(); // レイが輸送する寄与
    int bounce = 0; // レイが今まで何回反射したか
    float prob = 1.0f; // 反射確率（ロシアンルーレット打ち切り用）
    float ior = 1.0f; // レイが伝播している媒質の絶対屈折率
    RayTypes rayType = RayTypes::Camera; // レイの種類
};

} // namespace Core
} // namespace Petrichor
