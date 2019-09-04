#pragma once

#include <fmt/format.h>

namespace Petrichor
{
namespace Core
{

struct RenderSetting
{
    int outputWidth = 1280;       //!< 出力画像幅[px]
    int outputHeight = 720;       //!< 出力画像高さ[px]
    int numSamplesPerPixel = 128; //!< サンプル数
    int numSppForDenoising = 128; //!< デノイズ用出力のサンプル数
    int numMaxBounces = 16;       //!< maximum number of ray bounces
    int tileWidth = 16;           //!< tile width
    int tileHeight = 16;          //!< tile height

    //! number of render threads (0: use max number of threads)
    int numThreads = 0;
};

} // namespace Core
} // namespace Petrichor

template<>
struct fmt::formatter<Petrichor::Core::RenderSetting>
{
    template<typename ParseContext>
    constexpr auto
    parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const Petrichor::Core::RenderSetting& input, FormatContext& ctx)
    {
        return format_to(ctx.out(),
                         "OutputWidth: {}\n"
                         "OutputHeight: {}\n"
                         "NumSamplesPerPixel: {}\n"
                         "NumSppForDenoising: {}\n"
                         "NumMaxBounces: {}\n"
                         "TileWidth: {}\n"
                         "TileHeight: {}\n"
                         "NumThreads: {}\n",
                         input.outputWidth,
                         input.outputHeight,
                         input.numSamplesPerPixel,
                         input.numSppForDenoising,
                         input.numMaxBounces,
                         input.tileWidth,
                         input.tileHeight,
                         input.numThreads);
    }
};
