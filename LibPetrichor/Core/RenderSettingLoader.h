#pragma once

#include "Core/Assert.h"
#include "RenderSetting.h"
#include <filesystem>

namespace Petrichor
{
namespace Core
{

class ISceneSettingsLoader
{
public:
    virtual RenderSetting
    Load(const std::filesystem::path& path) = 0;
};

class RenderSettingLoaderJson : public ISceneSettingsLoader
{
public:
    RenderSetting
    Load(const std::filesystem::path& path) override;
};

class SceneSettingsTomlLoader : public ISceneSettingsLoader
{
public:
    RenderSetting
    Load(const std::filesystem::path& path) override;
};

} // namespace Core
} // namespace Petrichor
