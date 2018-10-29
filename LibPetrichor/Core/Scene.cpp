#include "Scene.h"

namespace Petrichor
{
namespace Core
{
void
Scene::LoadSceneSettings(const std::filesystem::path& path)
{
    SceneSettingsLoader loader;
    m_sceneSetting =
      loader.Load(path.c_str(), SceneSettingsLoader::FileType::Json);
}
} // namespace Core
} // namespace Petrichor
