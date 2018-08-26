#pragma once

#include "Core/Scene.h"
#include <string>
#include <chrono>

namespace Petrichor
{

struct RenderingResult
{
    float totalSec;
};

namespace Core
{

using ClockType = std::chrono::high_resolution_clock;


class Petrichor
{
public:
    void
    Initialize();

    void Render();

    void
    SaveImage(const std::string& path);

    

    void
    SetRenderCallback(
        const std::function<void(const RenderingResult&)>& onRenderingFinished
    )
    {
        m_onRenderingFinished = onRenderingFinished;
    }

private:
    void
    Finalize();

private:
    Scene m_scene;

    ClockType::time_point m_timeRenderingBegin;

    //! レンダリング終了時に呼ばれる
    std::function<void(const RenderingResult&)> m_onRenderingFinished;
};

} // namespace Core
} // namespace Petrichor
