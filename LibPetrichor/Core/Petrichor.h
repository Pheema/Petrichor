#pragma once

#include "Core/Scene.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <string>

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
    //! シーンをレンダリングする
    //! @param scene レンダリングするシーン
    void
    Render(const Scene& scene);

    void
    SetRenderCallback(
      const std::function<void(const RenderingResult&)>& onRenderingFinished)
    {
        m_onRenderingFinished = onRenderingFinished;
    }

    //! レンダリング済みタイルの個数を取得する。
    uint32_t
    GetNumRenderedTiles() const
    {
        return m_numRenderedTiles;
    }

    uint32_t
    GetNumTiles() const
    {
        return m_numTiles;
    }

private:
    void
    Finalize();

private:
    //! レンダリング済みタイルの個数
    std::atomic<uint32_t> m_numRenderedTiles = 0;

    //! レンダリング時間計測用
    ClockType::time_point m_timeRenderingBegin{};

    //! レンダリング終了時に呼ばれる
    std::function<void(const RenderingResult&)> m_onRenderingFinished;

    //!
    uint32_t m_numTiles = 0;
};

} // namespace Core
} // namespace Petrichor
