#include "SceneSettings.h"
#include <string>

namespace Petrichor
{
namespace Core
{

template<class T>
void
ReadValueIfKeyExists(T* val, const std::string& key, const json& json)
{
    const auto iter = json.find(key);
    if (iter != json.cend())
    {
        *val = static_cast<T>(*iter);
        return;
    }

    std::cout << "SceneSettings: key (" << key << ") is not found."
              << std::endl;
}

SceneSettings
SceneSettingsLoader::Load(const std::filesystem::path& path, FileType fileType)
{
    SceneSettings sceneSettings;

    switch (fileType)
    {
    case FileType::Json:
    {
        std::ifstream file(path, std::ios::in);
        if (file.fail())
        {
            std::cout << "Couldn't read the setting file" << std::endl;
            break;
        }

        json sceneSettingsJson;
        file >> sceneSettingsJson;
        file.close();

        ReadValueIfKeyExists(
          &sceneSettings.outputWidth, "outputWidth", sceneSettingsJson);
        ReadValueIfKeyExists(
          &sceneSettings.outputHeight, "outputHeight", sceneSettingsJson);
        ReadValueIfKeyExists(
          &sceneSettings.numSamplesPerPixel, "spp", sceneSettingsJson);
        ReadValueIfKeyExists(
          &sceneSettings.numMaxBouces, "maxBounces", sceneSettingsJson);
        ReadValueIfKeyExists(
          &sceneSettings.tileWidth, "tileWidth", sceneSettingsJson);
        ReadValueIfKeyExists(
          &sceneSettings.tileHeight, "tileHeight", sceneSettingsJson);
        ReadValueIfKeyExists(
          &sceneSettings.numThreads, "numThreads", sceneSettingsJson);

        break;
    }

    default:
    {
        ASSERT(false && "Unkown file types");
        break;
    }
    }

    return sceneSettings;
}
} // namespace Core
} // namespace Petrichor
