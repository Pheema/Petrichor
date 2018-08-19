#pragma once

#include "Core/Assert.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
using json = nlohmann::json;

namespace Petrichor
{
namespace Core
{

struct SceneSettings
{
    uint32_t outputWidth = 1280;       //!< rendered image width
    uint32_t outputHeight = 720;       //!< rendered image height
    uint32_t numSamplesPerPixel = 128; //!< number of samples in a pixel
    uint32_t numMaxBouces = 16;        //!< maximum number of ray bounces
    uint32_t tileWidth = 16;           //!< tile width
    uint32_t tileHeight = 16;          //!< tile height

    //! number of render threads (0: use max number of threads)
    uint32_t numThreads = 0;
};

class SceneSettingsLoader
{
public:
    enum class FileType
    {
        Json
    };

public:
    SceneSettings
    Load(const std::filesystem::path& path, FileType fileType);
};

} // namespace Core
} // namespace Petrichor
