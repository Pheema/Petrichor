#include "Petrichor.h"

#include "Core/Camera.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/Triangle.h"
#include "Core/Geometry/Vertex.h"
#include "Core/Integrator/PathTracing.h"
#include "Core/Integrator/SimplePathTracing.h"
#include "Core/Material/Emission.h"
#include "Core/Material/GGX.h"
#include "Core/Material/Lambert.h"
#include "Core/Material/MatMix.h"
#include "Core/Sampler/MicroJitteredSampler.h"
#include "Core/Sampler/RandomSampler1D.h"
#include "Core/Sampler/RandomSampler2D.h"
#include "Core/TileManager.h"
#include "Random/XorShift.h"
#include "Thread/ThreadPool.h"
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace Petrichor
{
namespace Core
{

void
Petrichor::Render(const Scene& scene)
{
    m_timeRenderingBegin = ClockType::now();

    // #TODO: 外部から設定可能にする
    SimplePathTracing pt;

    // #TODO 外部から設定可能にする
    // BruteForce accel;
    BinnedSAHBVH accel;
    accel.Build(scene);

    const uint32_t tileWidth = scene.GetRenderSetting().tileWidth;
    const uint32_t tileHeight = scene.GetRenderSetting().tileHeight;

    Texture2D* const targetTexure = scene.GetTargetTexture();
    if (targetTexure == nullptr)
    {
        std::cerr << "[Error] Target texture has not been set." << std::endl;
        return;
    }
    const uint32_t outputWidth = targetTexure->GetWidth();
    const uint32_t outputHeight = targetTexure->GetHeight();
    TileManager tileManager(outputWidth, outputHeight, tileWidth, tileHeight);
    m_numTiles = tileManager.GetNumTiles();

    const uint32_t numThreads = scene.GetRenderSetting().numThreads;

    m_numRenderedTiles = 0;
    {
        uint32_t maxIdxTile = 0;
        ThreadPool threadPool(numThreads);

        for (int idxTile = 0;
             idxTile < static_cast<int>(tileManager.GetNumTiles());
             idxTile++)
        {
            threadPool.Run([&](size_t threadIndex) {
                const Tile tile = tileManager.GetTile();
                const auto [i0, j0] = tile.GetInitialPixel();

                RandomSampler1D sampler1D(idxTile);
                RandomSampler2D sampler2D(idxTile, idxTile + 123456u);

                for (uint32_t j = j0; j < j0 + tile.GetHeight(); j++)
                {
                    for (uint32_t i = i0; i < i0 + tile.GetWidth(); i++)
                    {
                        pt.Render(i,
                                  j,
                                  scene,
                                  accel,
                                  targetTexure,
                                  sampler1D,
                                  sampler2D);
                    }
                }

                m_numRenderedTiles++;
            });
        }
    }
    Finalize();
}

void
Petrichor::Finalize()
{
    const std::chrono::duration<float> totalTime =
      ClockType::now() - m_timeRenderingBegin;

    {
        RenderingResult renderingResult;
        renderingResult.totalSec = totalTime.count();
        m_onRenderingFinished(renderingResult);
    }
}

} // namespace Core
} // namespace Petrichor
