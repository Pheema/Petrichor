#pragma once

#include "Core/Scene.h"
#include <chrono>
#include <string>
#include <vector>

namespace Petrichor
{
namespace Core
{

class Petrichor
{
public:
    void
    Initialize();

    void
    Render();

    void
    SaveImage(const std::string& path);

    void
    Finalize();

private:
    Scene m_scene;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_timeBegin;
};

} // namespace Core
} // namespace Petrichor
