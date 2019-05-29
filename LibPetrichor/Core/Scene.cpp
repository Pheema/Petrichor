#include "Scene.h"
#include "Core/RenderSettingLoader.h"

#include "fmt/format.h"

namespace Petrichor
{
namespace Core
{
void
Scene::LoadRenderSetting(const std::filesystem::path& path)
{
    auto loader = new RenderSettingLoaderJson();
    m_renderSetting = loader->Load(path.c_str());

    fmt::print("[Setting]\n{}\n", m_renderSetting);
}
} // namespace Core
} // namespace Petrichor
