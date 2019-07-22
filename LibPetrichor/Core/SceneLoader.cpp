#include "SceneLoader.h"

#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include <fstream>

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

    {
        auto iter = loadedJson.find("assets");
        for (; iter != loadedJson.cend(); iter++)
        {
            auto paths = iter->get<std::vector<std::string>>();
            for (const auto& path : paths)
            {
                scene.LoadModel(path);
            }
        }
    }
}

} // namespace Core
} // namespace Petrichor
