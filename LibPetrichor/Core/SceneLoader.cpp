#include "SceneLoader.h"

#include "Core/Material/Lambert.h"
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

    // ---- Material ----
    {
        {
            auto defaultMat = std::make_unique<Lambert>(Color3f::One());
            scene.RegisterMaterial("default", std::move(defaultMat));
        }

        auto materials = loadedJson["materials"];
        for (const auto& material : materials)
        {
            std::string materialType;
            loadValue(&materialType, material, "type");

            if (materialType.empty())
            {
                continue;
            }

            std::string materialName;
            loadValue(&materialName, material, "name");
            if (materialName.empty())
            {
                continue;
            }

            if (materialType == "lambert")
            {
                const auto baseColor = loadVector3f(material, "base_color");
                auto lambertMat = std::make_unique<Lambert>(baseColor);

                std::string colorTexPath;
                loadValue(&colorTexPath, material, "color_tex");
                if (!colorTexPath.empty())
                {
                    Texture2D colorTex;
                    if (colorTex.Load(colorTexPath,
                                      Texture2D::TextureColorType::Color))
                    {
                        const TextureHandle colorTexHandle =
                          scene.RegisterTexture(colorTex);

                        lambertMat->SetTexAlbedo(
                          &scene.GetTexture(colorTexHandle));
                    }
                }

                scene.RegisterMaterial(materialName, std::move(lambertMat));
            }
            else if (materialType == "ggx")
            {
            }
            else if (materialType == "emission")
            {
            }
            else
            {
                fmt::print("[SceneLoaderJson] Invalid material type '{}'.",
                           materialType);
            }
        }
    }

    // ---- Camera ----
    {
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

        {
            float focalLength = 55e-3f;
            loadValue(&focalLength, cameraData, "focal_length");
            camera->SetFocalLength(focalLength);
        }

        scene.SetMainCamera(std::move(camera));
    }

    // ---- env ---
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

    // ---- assets ----
    {
        const auto assets = loadedJson["assets"];
        for (const auto& asset : assets)
        {
            if (asset.find("mat") != asset.cend())
            {
                scene.LoadModel(asset["path"].get<std::string>(),
                                asset["mat"].get<std::string>());
            }
            else
            {
                scene.LoadModel(asset["path"].get<std::string>());
            }
        }
    }
} // namespace Core

} // namespace Core
} // namespace Petrichor
