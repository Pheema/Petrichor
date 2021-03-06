#include "Petrichor.h"

#include "Core/AOV/AOVDenoisingAlbedo.h"
#include "Core/AOV/AOVDenoisingNormal.h"
#include "Core/AOV/AOVUVCoordinate.h"
#include "Core/Camera.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/Triangle.h"
#include "Core/Geometry/Vertex.h"
#include "Core/Integrator/PathTracing.h"
#include "Core/Integrator/SimplePathTracing.h"
#include "Core/Logger.h"
#include "Core/Material/Emission.h"
#include "Core/Material/GGX.h"
#include "Core/Material/Lambert.h"
#include "Core/Material/MixMaterial.h"
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
    SCOPE_LOGGER(__FUNCTION__);

    // #TODO 外部から設定可能にする
    // BruteForce accel;
    const BinnedSAHBVH accel = [&] {
        BinnedSAHBVH accel_;
        accel_.Build(scene);
        return accel_;
    }();

    const uint32_t tileWidth = scene.GetRenderSetting().tileWidth;
    const uint32_t tileHeight = scene.GetRenderSetting().tileHeight;

    {
        Texture2D* const targetTexure =
          scene.GetTargetTexture(Scene::AOVType::Rendered);
        if (targetTexure)
        {
            SCOPE_LOGGER("[AOV] Rendered");

            // #TODO: 外部から設定可能にする
            SimplePathTracing pt;

            const int outputWidth = targetTexure->GetWidth();
            const int outputHeight = targetTexure->GetHeight();
            const TileManager tileManager(
              outputWidth, outputHeight, tileWidth, tileHeight);
            m_numTiles = tileManager.GetNumTiles();

            m_numRenderedTiles = 0;

            {
                const uint32_t numThreads = scene.GetRenderSetting().numThreads;
                ThreadPool threadPool(numThreads);

                int tileIndex = 0;
                for (const TileManager::Tile& tile : tileManager.GetTiles())
                {
                    threadPool.Push([&, tile, tileIndex](size_t threadIndex) {
                        RandomSampler1D sampler1D(tileIndex);
                        RandomSampler2D sampler2D(tileIndex, tileIndex + 1);

                        for (int y = tile.y; y < tile.y + tile.height; y++)
                        {
                            for (int x = tile.x; x < tile.x + tile.width; x++)
                            {
                                pt.Render(x,
                                          y,
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
        }
        else
        {
            Logger::Error("Target texture has not been set.\n");
        }
    }

    // UV
    {
        Texture2D* const uvCoordinateTexture =
          scene.GetTargetTexture(Scene::AOVType::UV);
        if (uvCoordinateTexture)
        {
            SCOPE_LOGGER("[AOV] UV");

            AOVUVCoordinate renderer;

            const uint32_t tileWidth = scene.GetRenderSetting().tileWidth;
            const uint32_t tileHeight = scene.GetRenderSetting().tileHeight;

            const int outputWidth = uvCoordinateTexture->GetWidth();
            const int outputHeight = uvCoordinateTexture->GetHeight();
            const TileManager tileManager(
              outputWidth, outputHeight, tileWidth, tileHeight);

            m_numRenderedTiles = 0;

            {
                const uint32_t numThreads = scene.GetRenderSetting().numThreads;
                ThreadPool threadPool(numThreads);

                int tileIndex = 0;
                for (const TileManager::Tile& tile : tileManager.GetTiles())
                {
                    threadPool.Push([&, tile, tileIndex](size_t threadIndex) {
                        RandomSampler1D sampler1D(tileIndex);
                        RandomSampler2D sampler2D(tileIndex, tileIndex + 1);

                        for (int y = tile.y; y < tile.y + tile.height; y++)
                        {
                            for (int x = tile.x; x < tile.x + tile.width; x++)
                            {
                                renderer.Render(x,
                                                y,
                                                scene,
                                                accel,
                                                uvCoordinateTexture,
                                                sampler1D,
                                                sampler2D);
                            }
                        }

                        m_numRenderedTiles++;
                    });
                    tileIndex++;
                }
            }
        }
    }

    // Albedo
    {
        Texture2D* const denoisingAlbedoTexture =
          scene.GetTargetTexture(Scene::AOVType::DenoisingAlbedo);
        if (denoisingAlbedoTexture)
        {
            SCOPE_LOGGER("[AOV] DenoisingAlbedo");

            AOVDenoisingAlbedo renderer;

            const uint32_t tileWidth = scene.GetRenderSetting().tileWidth;
            const uint32_t tileHeight = scene.GetRenderSetting().tileHeight;

            const int outputWidth = denoisingAlbedoTexture->GetWidth();
            const int outputHeight = denoisingAlbedoTexture->GetHeight();
            const TileManager tileManager(
              outputWidth, outputHeight, tileWidth, tileHeight);

            m_numRenderedTiles = 0;

            {
                const uint32_t numThreads = scene.GetRenderSetting().numThreads;
                ThreadPool threadPool(numThreads);

                int tileIndex = 0;
                for (const TileManager::Tile& tile : tileManager.GetTiles())
                {
                    threadPool.Push([&, tile, tileIndex](size_t threadIndex) {
                        RandomSampler1D sampler1D(tileIndex);
                        RandomSampler2D sampler2D(tileIndex, tileIndex + 1);

                        for (int y = tile.y; y < tile.y + tile.height; y++)
                        {
                            for (int x = tile.x; x < tile.x + tile.width; x++)
                            {
                                renderer.Render(x,
                                                y,
                                                scene,
                                                accel,
                                                denoisingAlbedoTexture,
                                                sampler1D,
                                                sampler2D);
                            }
                        }

                        m_numRenderedTiles++;
                    });
                    tileIndex++;
                }
            }
        }
    }

    // WorldNormal
    {
        Texture2D* const aovWorldNormalTexture =
          scene.GetTargetTexture(Scene::AOVType::DenoisingNormal);
        if (aovWorldNormalTexture)
        {
            SCOPE_LOGGER("[AOV] DenoisingNormal");

            AOVDenoisingNormal renderer;

            const uint32_t tileWidth = scene.GetRenderSetting().tileWidth;
            const uint32_t tileHeight = scene.GetRenderSetting().tileHeight;

            const int outputWidth = aovWorldNormalTexture->GetWidth();
            const int outputHeight = aovWorldNormalTexture->GetHeight();
            const TileManager tileManager(
              outputWidth, outputHeight, tileWidth, tileHeight);

            m_numRenderedTiles = 0;

            {
                const uint32_t numThreads = scene.GetRenderSetting().numThreads;
                ThreadPool threadPool(numThreads);

                int tileIndex = 0;
                for (const TileManager::Tile& tile : tileManager.GetTiles())
                {
                    threadPool.Push([&, tile, tileIndex](size_t threadIndex) {
                        RandomSampler1D sampler1D(tileIndex);
                        RandomSampler2D sampler2D(tileIndex, tileIndex + 1);

                        for (int y = tile.y; y < tile.y + tile.height; y++)
                        {
                            for (int x = tile.x; x < tile.x + tile.width; x++)
                            {
                                renderer.Render(x,
                                                y,
                                                scene,
                                                accel,
                                                aovWorldNormalTexture,
                                                sampler1D,
                                                sampler2D);
                            }
                        }

                        m_numRenderedTiles++;
                    });
                    tileIndex++;
                }
            }
        }
    }

    Finalize();
}

void
Petrichor::Finalize()
{
    if (m_onRenderingFinished)
    {
        RenderingResult renderingResult;
        m_onRenderingFinished(renderingResult);
    }
}

} // namespace Core
} // namespace Petrichor
