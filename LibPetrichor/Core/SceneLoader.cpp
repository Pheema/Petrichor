#include "SceneLoader.h"

#include "Core/Logger.h"
#include "Core/Material/GGX.h"
#include "Core/Material/Glass.h"
#include "Core/Material/Lambert.h"
#include "Core/Material/MixMaterial.h"
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
            Logger::Error("Failed to read the scene file. [{}]", path.string());
            return nlohmann::json{};
        }

        nlohmann::json ret;
        file >> ret;
        return ret;
    }();

    Logger::Info("Scene file loaded. [{}]", path.string());

    auto loadVector3f = [](const nlohmann::json& json, const char* arrayName) {
        if (json.find(arrayName) != json.cend())
        {
            auto loadedVector = json[arrayName].get<std::vector<float>>();

            if (loadedVector.size() != 3)
            {
                Logger::Error("[SceneLoaderJson] '{}' size != 3.", arrayName);
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
            Logger::Error("[SceneLoaderJson] '{}' not found.", arrayName);
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
            Logger::Error("[SceneLoaderJson] '{}' not found.", keyName);
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

                std::string colorTexturePathString;
                loadValue(&colorTexturePathString, material, "color_tex");

                if (!colorTexturePathString.empty())
                {
                    std::filesystem::path colorTexturePath =
                      path.parent_path() / colorTexturePathString;

                    Texture2D colorTex;
                    if (colorTex.Load(colorTexturePath,
                                      Texture2D::TextureColorType::Color))
                    {
                        const TextureHandle colorTexHandle =
                          scene.RegisterTexture(colorTexturePath, colorTex);

                        lambertMat->SetTexAlbedo(
                          scene.GetTexture(colorTexHandle));
                    }
                }

                scene.RegisterMaterial(materialName, std::move(lambertMat));
            }
            else if (materialType == "glass")
            {
                const auto color = loadVector3f(material, "color");

                float ior = 1.0f;
                loadValue(&ior, material, "ior");

                auto glassMat = std::make_unique<Glass>(color, ior);

                scene.RegisterMaterial(materialName, std::move(glassMat));
            }
            else if (materialType == "glossy")
            {
                const auto color = loadVector3f(material, "color");

                float roughness = 0.0f;
                loadValue(&roughness, material, "roughness");

                auto ggxMat = std::make_unique<GGX>(color, roughness);

                std::string colorTexturePathString;
                loadValue(&colorTexturePathString, material, "color_tex");
                if (!colorTexturePathString.empty())
                {
                    const std::filesystem::path colorTexturePath =
                      colorTexturePathString;

                    Texture2D colorTexture;
                    if (colorTexture.Load(path.parent_path() / colorTexturePath,
                                          Texture2D::TextureColorType::Color))
                    {
                        const TextureHandle textureHandle =
                          scene.RegisterTexture(colorTexturePath, colorTexture);

                        ggxMat->SetF0Texture(scene.GetTexture(textureHandle));
                    }
                }

                std::string roughnessTexturePath;
                loadValue(&roughnessTexturePath, material, "roughness_tex");
                if (!roughnessTexturePath.empty())
                {
                    Texture2D roughnessTexture;
                    if (roughnessTexture.Load(
                          roughnessTexturePath,
                          Texture2D::TextureColorType::NonColor))
                    {
                        const TextureHandle textureHandle =
                          scene.RegisterTexture(roughnessTexturePath,
                                                roughnessTexture);

                        ggxMat->SetRoughnessMap(
                          scene.GetTexture(textureHandle));
                    }
                }

                scene.RegisterMaterial(materialName, std::move(ggxMat));
            }
            else if (materialType == "emission")
            {
            }
            else if (materialType == "mix")
            {
                std::string mat0Name;
                loadValue(&mat0Name, material, "mat0_name");
                const auto mat0 = scene.GetMaterial(mat0Name);

                std::string mat1Name;
                loadValue(&mat1Name, material, "mat1_name");
                const auto mat1 = scene.GetMaterial(mat1Name);

                float mix = 0.5;
                loadValue(&mix, material, "mix");

                auto mixedMat = std::make_unique<MixMaterial>(mat0, mat1, mix);
                scene.RegisterMaterial(materialName, std::move(mixedMat));
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
            std::string texturePathString("");
            loadValue(&texturePathString, envData, "texture");
            if (!texturePathString.empty())
            {
                const std::filesystem::path texturePath =
                  path.parent_path() / texturePathString;
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
                scene.LoadModel(path.parent_path() /
                                  asset["path"].get<std::string>(),
                                asset["mat"].get<std::string>());
            }
            else
            {
                scene.LoadModel(path.parent_path() /
                                asset["path"].get<std::string>());
            }
        }
    }
} // namespace Core

} // namespace Core
} // namespace Petrichor
