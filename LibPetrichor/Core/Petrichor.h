#pragma once

#include "Core/Scene.h"
#include <string>

namespace Petrichor
{
namespace Core
{

class Petrichor
{
public:
    void
    Initialize();

    void Render();

    void
    SaveImage(const std::string& path);

    void
    Finalize();

private:
    Scene m_scene;
};

} // namespace Core
} // namespace Petrichor
