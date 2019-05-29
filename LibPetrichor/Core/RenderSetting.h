#pragma once

#include "fmt/format.h"

namespace Petrichor
{
namespace Core
{

struct RenderSetting
{
    int outputWidth = 1280;       //!< rendered image width
    int outputHeight = 720;       //!< rendered image height
    int numSamplesPerPixel = 128; //!< number of samples in a pixel
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
                         "NumMaxBounces: {}\n"
                         "TileWidth: {}\n"
                         "TileHeight: {}\n"
                         "NumThreads: {}\n",
                         input.outputWidth,
                         input.outputHeight,
                         input.numSamplesPerPixel,
                         input.numMaxBounces,
                         input.tileWidth,
                         input.tileHeight,
                         input.numThreads);
    }
};
