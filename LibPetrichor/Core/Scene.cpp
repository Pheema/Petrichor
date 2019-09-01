#include "Scene.h"
#include "Core/RenderSettingLoader.h"
#include "Material/Lambert.h"
#include "SceneLoader.h"
#include <memory>

namespace Petrichor
{
namespace Core
{
void
Scene::LoadRenderSetting(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        fmt::print("Render setting file is not found.\n{}\n", path.string());
        return;
    }

    auto loader = std::make_unique<RenderSettingLoaderJson>();
    m_renderSetting = loader->Load(path.c_str());

    fmt::print("[Setting]\n{}\n", m_renderSetting);
}

void
Scene::LoadModel(const std::filesystem::path& path)
{
    // #TODO: 生newやめる
    // #TODO: マテリアルのアサインをどこにするか考える。
    auto const mesh = new Mesh();
    auto const lambert = new Lambert(Color3f::One());
    mesh->Load(path, lambert, ShadingTypes::Smooth);
    AppendMesh(*mesh);
}

void
Scene::LoadAssets(const std::filesystem::path path)
{
    auto sceneLoader = std::make_unique<SceneLoaderJson>();
    sceneLoader->Load(path, *this);
}

} // namespace Core
} // namespace Petrichor
