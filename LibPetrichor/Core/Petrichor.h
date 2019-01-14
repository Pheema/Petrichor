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
    int
    GetNumRenderedTiles() const
    {
        return m_numRenderedTiles;
    }

    //! タイルの個数を取得する。
    int
    GetNumTiles() const
    {
        return m_numTiles;
    }

private:
    void
    Finalize();

private:
    //! レンダリング済みタイルの個数
    std::atomic<int> m_numRenderedTiles = 0;

    //! タイルの最大個数
    int m_numTiles = 0;

    //! レンダリング時間計測用
    ClockType::time_point m_timeRenderingBegin{};

    //! レンダリング終了時に呼ばれる
    std::function<void(const RenderingResult&)> m_onRenderingFinished;
};

} // namespace Core
} // namespace Petrichor
