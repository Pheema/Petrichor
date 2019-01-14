#pragma once

namespace Petrichor
{
namespace Core
{

struct RenderSetting
{
    int outputWidth = 1280;       //!< rendered image width
    int outputHeight = 720;       //!< rendered image height
    int numSamplesPerPixel = 128; //!< number of samples in a pixel
    int numMaxBouces = 16;        //!< maximum number of ray bounces
    int tileWidth = 16;           //!< tile width
    int tileHeight = 16;          //!< tile height

    //! number of render threads (0: use max number of threads)
    int numThreads = 0;
};

} // namespace Core
} // namespace Petrichor
