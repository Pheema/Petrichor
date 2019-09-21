#include "RenderSettingLoader.h"
#include "Core/Logger.h"
#include "nlohmann/json.hpp"
#include "toml/toml.h"
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>

using Json = nlohmann::json;

namespace Petrichor
{
namespace Core
{

RenderSetting
RenderSettingLoaderJson::Load(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in);
    if (file.fail())
    {
        Logger::Error("Failed to read the setting file");
        return RenderSetting{};
    }

    RenderSetting renderSetting;

    Json renderSettingJson;
    file >> renderSettingJson;
    file.close();

    Logger::Info("Render setting file loaded. [{}]", path.string());

    auto readValueIfKeyExists =
      [](auto* result, const std::string& key, const Json& json) -> void {
        using ValueType = typename std::remove_pointer<decltype(result)>::type;

        const auto iter = json.find(key);
        if (iter != json.cend())
        {
            *result = static_cast<ValueType>(*iter);
            return;
        }

        Logger::Error("RenderSetting: key is not found. [{}]", key);
    };

    readValueIfKeyExists(
      &renderSetting.outputWidth, "outputWidth", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.outputHeight, "outputHeight", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numSamplesPerPixel, "spp", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numSppForDenoising, "denoising_spp", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numMaxBounces, "maxBounces", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.tileWidth, "tileWidth", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.tileHeight, "tileHeight", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numThreads, "numThreads", renderSettingJson);

    return renderSetting;
}

RenderSetting
SceneSettingsTomlLoader::Load(const std::filesystem::path& path)
{
    RenderSetting sceneSettings;

    std::ifstream file(path, std::ios::in);
    if (file.fail())
    {
        Logger::Error("Failed to read the setting file");
        return sceneSettings;
    }

    const toml::ParseResult parsedData = toml::parse(file);
    if (!parsedData.valid())
    {
        Logger::Error("Failed to parse the toml file.");
        return sceneSettings;
    }

    auto readValueIfKeyExists =
      [](auto* result,
         const std::string& key,
         const toml::ParseResult& parsedData) -> void {
        using ValueType = typename std::remove_pointer<decltype(result)>::type;

        const toml::Value* value = parsedData.value.find(key);
        if (value && value->is<ValueType>())
        {
            *result = value->as<ValueType>();
        }
        else
        {
            Logger::Error("RenderSetting: key is not found. [{}]", key);
        }
    };

    readValueIfKeyExists(
      &sceneSettings.outputWidth, "settings.render_width", parsedData);

    readValueIfKeyExists(
      &sceneSettings.outputHeight, "settings.render_height", parsedData);

    readValueIfKeyExists(
      &sceneSettings.numSamplesPerPixel, "settings.spp", parsedData);

    readValueIfKeyExists(
      &sceneSettings.numMaxBounces, "settings.maxBounces", parsedData);

    readValueIfKeyExists(
      &sceneSettings.tileWidth, "settings.tile_width", parsedData);

    readValueIfKeyExists(
      &sceneSettings.tileHeight, "settings.tile_height", parsedData);

    readValueIfKeyExists(
      &sceneSettings.numThreads, "settings.num_threads", parsedData);

    return sceneSettings;
}

} // namespace Core
} // namespace Petrichor
