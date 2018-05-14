#pragma once

#include "Cereal/archives/json.hpp"

namespace Petrichor
{
namespace Core
{

struct SceneSettings
{
    uint32_t outputWidth     = 1280; // レンダリング画面幅[px]
    uint32_t outputHeight    = 720;  // レンダリング画面高さ[px]
    uint32_t samplesPerPixel = 64;   // 1ピクセル毎のサンプル数
    uint32_t tileWidth       = 32;   // レンダリングタイルの幅[px]
    uint32_t tileHeight      = 32;   // レンダリングタイルの高さ[px]
    uint32_t maxNumBounces   = 64;   // レイの反射数上限[px]

    template<class Archive>
    void
    serialize(Archive& ar)
    {
        ar(CEREAL_NVP(outputWidth),
           CEREAL_NVP(outputHeight),
           CEREAL_NVP(samplesPerPixel),
           CEREAL_NVP(tileWidth),
           CEREAL_NVP(tileHeight),
           CEREAL_NVP(maxNumBounces));
    }
};

} // namespace Core
} // namespace Petrichor
