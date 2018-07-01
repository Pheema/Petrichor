#include "Scene.h"

namespace Petrichor
{
namespace Core
{
void
Scene::LoadSceneSettings()
{
    SceneSettingsLoader loader;
    m_sceneSetting =
      loader.Load("settings.json", SceneSettingsLoader::FileType::Json);
}
}
}
