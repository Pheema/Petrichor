#include "SceneLoader.h"

#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <memory>

namespace Petrichor
{
namespace Core
{
void
SceneLoaderJson::Load(const std::filesystem::path& path, Scene& scene)
{
    const nlohmann::json loadedJson = [&]() {
        std::ifstream file(path, std::ios::in);
        if (file.fail())
        {
            fmt::print("Couldn't read the scene file\n({})\n", path.string());
            return nlohmann::json{};
        }

        nlohmann::json ret;
        file >> ret;
        return ret;
    }();

    auto loadVector3f = [](const nlohmann::json& json, const char* arrayName) {
        if (json.find(arrayName) != json.cend())
        {
            auto loadedVector = json[arrayName].get<std::vector<float>>();

            if (loadedVector.size() != 3)
            {
                fmt::print("[SceneLoaderJson] '{}' size != 3.", arrayName);
            }

            Math::Vector3f v;
            const int arraySize =
              std::min(3, static_cast<int>(loadedVector.size()));
            for (int i = 0; i < arraySize; i++)
            {
                v[i] = loadedVector[i];
            }

            return v;
        }
        else
        {
            fmt::print("[SceneLoaderJson] '{}' not found.", arrayName);
            return Math::Vector3f::Zero();
        }
    };

    auto loadValue = [](auto* const result,
                        const nlohmann::json& json,
                        const char* keyName) -> void {
        using ValueType = typename std::remove_pointer<decltype(result)>::type;

        if (json.find(keyName) != json.cend())
        {
            const auto loadedValue = json[keyName].get<ValueType>();
            if (result)
            {
                *result = loadedValue;
            }
        }
        else
        {
            fmt::print("[SceneLoaderJson] '{}' not found.", keyName);
        }
    };

    {
        // #TODO: unique_ptr使ってシーンに所有権を渡すとか？
        // #TODO: エラー処理
        auto camera = std::make_unique<Camera>();

        const auto cameraData = loadedJson["camera"];

        {
            const auto position = loadVector3f(cameraData, "position");
            camera->SetPosition(position);
        }

        {
            const auto lookAt = loadVector3f(cameraData, "look_at");
            camera->LookAt(lookAt);
        }

        {
            const auto focusPos = loadVector3f(cameraData, "focus_pos");
            camera->FocusTo(focusPos);
        }

        {
            float fNumber = 2.8f;
            loadValue(&fNumber, cameraData, "f_number");
            camera->SetFNumber(fNumber);
        }

        scene.SetMainCamera(std::move(camera));
    }

    {
        const auto envData = loadedJson["env"];

        Environment env;

        {
            const auto baseColor = loadVector3f(envData, "base_color");
            env.SetBaseColor(baseColor);
        }

        {
            std::string texturePath("");
            loadValue(&texturePath, envData, "texture");
            if (!texturePath.empty())
            {
                env.Load(texturePath);
            }
        }

        scene.SetEnvironment(env);
    }

    {
        const auto assetsData = loadedJson["assets"];
        auto paths = assetsData.get<std::vector<std::string>>();
        for (const auto& path : paths)
        {
            scene.LoadModel(path);
        }
    }
}

} // namespace Core
} // namespace Petrichor
