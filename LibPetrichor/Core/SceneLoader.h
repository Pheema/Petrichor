#pragma once

#include "Scene.h"
#include <filesystem>

namespace Petrichor
{
namespace Core
{

class ISceneLoader
{
public:
    virtual void
    Load(const std::filesystem::path& path, Scene& scene) = 0;
};

class SceneLoaderJson : public ISceneLoader
{
public:
    void
    Load(const std::filesystem::path& path, Scene& scene) override;
};

} // namespace Core
} // namespace Petrichor
