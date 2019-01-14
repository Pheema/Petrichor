#include "Scene.h"
#include "Core/RenderSettingLoader.h"

namespace Petrichor
{
namespace Core
{
void
Scene::LoadRenderSetting(const std::filesystem::path& path)
{
    auto loader = new RenderSettingLoaderJson();
    m_renderSetting = loader->Load(path.c_str());
}
} // namespace Core
} // namespace Petrichor
