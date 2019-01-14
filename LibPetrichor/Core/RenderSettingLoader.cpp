#include "RenderSettingLoader.h"
// #include "tinytoml/include/toml/toml.h"
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include "nlohmann/json.hpp"
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
        std::cout << "Couldn't read the setting file" << std::endl;
        return RenderSetting{};
    }

    RenderSetting renderSetting;

    Json renderSettingJson;
    file >> renderSettingJson;
    file.close();

    auto readValueIfKeyExists =
      [](auto* value, const std::string& key, const Json& json) -> void {
        const auto iter = json.find(key);
        if (iter != json.cend())
        {
            *value =
              static_cast<std::remove_pointer<decltype(value)>::type>(*iter);
            return;
        }

        std::cout << "RenderSetting: key (" << key << ") is not found."
                  << std::endl;
    };

    readValueIfKeyExists(
      &renderSetting.outputWidth, "outputWidth", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.outputHeight, "outputHeight", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numSamplesPerPixel, "spp", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numMaxBouces, "maxBounces", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.tileWidth, "tileWidth", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.tileHeight, "tileHeight", renderSettingJson);
    readValueIfKeyExists(
      &renderSetting.numThreads, "numThreads", renderSettingJson);

    return renderSetting;
}
//
// template<class T>
// void
// ReadTomlValueIfKeyExists(T* result,
//                         const std::string& key,
//                         const toml::ParseResult& parsedData)
//{
//    const toml::Value* value = parsedData.value.find(key);
//    if (value && value->is<T>())
//    {
//        *result = value->as<T>();
//    }
//    else
//    {
//        std::cout << "SceneSettings: key (" << key << ") is not found."
//                  << std::endl;
//    }
//}
//
// SceneSettings
// SceneSettingsTomlLoader::Load(const std::filesystem::path& path)
//{
//    SceneSettings sceneSettings;
//
//    std::ifstream file(path, std::ios::in);
//    if (file.fail())
//    {
//        std::cerr << "Couldn't read the setting file" << std::endl;
//        return sceneSettings;
//    }
//
//    const toml::ParseResult parsedData = toml::parse(file);
//    if (!parsedData.valid())
//    {
//        std::cerr << "Parsing toml file failed.\n";
//        return sceneSettings;
//    }
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.outputWidth, "settings.render_width", parsedData);
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.outputHeight, "settings.render_height", parsedData);
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.numSamplesPerPixel, "settings.spp", parsedData);
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.numMaxBouces, "settings.maxBounces", parsedData);
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.tileWidth, "settings.tile_width", parsedData);
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.tileHeight, "settings.tile_height", parsedData);
//
//    ReadTomlValueIfKeyExists(
//      &sceneSettings.numThreads, "settings.num_threads", parsedData);
//
//    return sceneSettings;
//}

} // namespace Core
} // namespace Petrichor
