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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace Petrichor
{
namespace Core
{

void
Petrichor::Initialize()
{

#if 1

    auto* baseColorTex = new Texture2D();
    baseColorTex->Load("Resource/RTCamp6/Textures/BaseColor.png",
                       Texture2D::TextureColorType::Color);
    auto* normalMap = new Texture2D();
    normalMap->Load("Resource/RTCamp6/CornellBox/MetalNormal.png",
                    Texture2D::TextureColorType::NonColor);

    const auto* const matLambertRed =
      m_scene.AppendMaterial(Lambert(Color3f(1.0f, 0.0f, 0.0f)));
    const auto* const matLambertGreen =
      m_scene.AppendMaterial(Lambert(Color3f(0, 1.0f, 0)));
    auto* const matLamberWhite =
      m_scene.AppendMaterial(Lambert(0.8f * Color3f::One()));

    matLamberWhite->SetTexAlbedo(baseColorTex);
    // MaterialBase* matLambertRed = new Lambert(Color3f(1.0f, 0, 0));
    // MaterialBase* matLambertGreen = new Lambert(Color3f(0, 1.0f, 0));
    // MaterialBase* matLamberWhite = new Lambert(Color3f::One());
    // GGX* matGGX = new GGX(0.9f * Color3f::One(), 0.1f);

    /*auto* const matGGX =
      m_scene.AppendMaterial(GGX(0.9f * Color3f::One(), 0.1f));
    matGGX->SetRoughnessMap(roughnessTex);
    matGGX->SetRoughnessMapStrength(0.3f);
    matGGX->SetNormalMap(normalMap);
    matGGX->SetNormalMapStrength(0.5f);*/

    // MaterialBase* matEmissionWhite = new Emission(10.0f * Color3f::One());

    const auto* const matEmissionWhite =
      m_scene.AppendMaterial(Emission(Color3f::One()));

    auto const room = new Mesh();
    room->Load("Resource/RTCamp6/room.obj", matLamberWhite, ShadingTypes::Flat);

    m_scene.AppendMesh(*room);

    auto const camera =
      new Camera(Math::Vector3f(0, 1.0f, 1.6f), Math::Vector3f::UnitY());

    camera->SetLens(36e-3f);
    camera->SetFNumber(6.0f);
    camera->FocusTo(Math::Vector3f(0, 12.0f, 0.3f));

    m_scene.LoadSceneSettings();
    m_scene.SetMainCamera(*camera);

    // 環境マップの設定
    m_scene.GetEnvironment().Load("Resource/RTCamp6/venice_sunset_2k.hdr");
    m_scene.GetEnvironment().SetBaseColor(Color3f::One());
    m_scene.GetEnvironment().SetZAxisRotation(-Math::kPi);

    // レンダリング先を指定
    auto targetTex = new Texture2D(m_scene.GetSceneSettings().outputWidth,
                                   m_scene.GetSceneSettings().outputHeight);
    m_scene.SetTargetTexture(targetTex);

#else

    auto* roughnessTex = new Texture2D();
    roughnessTex->Load("Resource/SampleScene/CornellBox/MetalRoughness.png",
                       Texture2D::TextureColorType::NonColor);
    auto* normalMap = new Texture2D();
    normalMap->Load("Resource/SampleScene/CornellBox/MetalNormal.png",
                    Texture2D::TextureColorType::NonColor);

    const auto* const matLambertRed =
      m_scene.AppendMaterial(Lambert(Color3f(1.0f, 0.0f, 0.0f)));
    const auto* const matLambertGreen =
      m_scene.AppendMaterial(Lambert(Color3f(0, 1.0f, 0)));
    const auto* const matLamberWhite =
      m_scene.AppendMaterial(Lambert(Color3f::One()));
    // MaterialBase* matLambertRed = new Lambert(Color3f(1.0f, 0, 0));
    // MaterialBase* matLambertGreen = new Lambert(Color3f(0, 1.0f, 0));
    // MaterialBase* matLamberWhite = new Lambert(Color3f::One());
    // GGX* matGGX = new GGX(0.9f * Color3f::One(), 0.1f);

    auto* const matGGX =
      m_scene.AppendMaterial(GGX(0.9f * Color3f::One(), 0.1f));
    matGGX->SetRoughnessMap(roughnessTex);
    matGGX->SetRoughnessMapStrength(0.3f);
    matGGX->SetNormalMap(normalMap);
    matGGX->SetNormalMapStrength(0.5f);

    // MaterialBase* matEmissionWhite = new Emission(10.0f * Color3f::One());

    const auto* const matEmissionWhite =
      m_scene.AppendMaterial(Emission(Color3f::One()));

    auto const leftWall = new Mesh();
    leftWall->Load("Resource/SampleScene/CornellBox/LeftWall.obj",
                   matLambertRed,
                   ShadingTypes::Flat);

    auto const rightWall = new Mesh();
    rightWall->Load("Resource/SampleScene/CornellBox/RightWall.obj",
                    matLambertGreen,
                    ShadingTypes::Flat);

    auto const whiteWall = new Mesh();
    whiteWall->Load("Resource/SampleScene/CornellBox/WhiteWall.obj",
                    matLamberWhite,
                    ShadingTypes::Flat);

    auto const whiteBox = new Mesh();
    whiteBox->Load("Resource/SampleScene/CornellBox/WhiteBox.obj",
                   matGGX,
                   ShadingTypes::Flat);

    auto const ceilLight = new Mesh();
    ceilLight->Load("Resource/SampleScene/CornellBox/CeilLight.obj",
                    matEmissionWhite,
                    ShadingTypes::Flat);

    m_scene.LoadSceneSettings();

    m_scene.AppendMesh(*leftWall);
    m_scene.AppendMesh(*rightWall);
    m_scene.AppendMesh(*whiteWall);
    m_scene.AppendMesh(*whiteBox);
    m_scene.AppendLightMesh(*ceilLight);

    auto const camera =
      new Camera(Math::Vector3f(0, -6.0f, 0), Math::Vector3f::UnitY());

    camera->FocusTo(Math::Vector3f::Zero());
    // camera->SetLens(18e-3f);

    m_scene.SetMainCamera(*camera);

    m_scene.LoadSceneSettings();

    // 環境マップの設定
    m_scene.GetEnvironment().Load(
      "Resource/SampleScene/CornellBox/cape_hill_2k.hdr");
    m_scene.GetEnvironment().SetBaseColor(Color3f::One());
    m_scene.GetEnvironment().SetZAxisRotation(-1.25f * Math::kPi);

    // レンダリング先を指定
    auto targetTex = new Texture2D(m_scene.GetSceneSettings().outputWidth,
                                   m_scene.GetSceneSettings().outputHeight);
    m_scene.SetTargetTexture(targetTex);

#endif
}

void
Petrichor::Render()
{
    m_timeRenderingBegin = ClockType::now();

    PathTracing pt;

    const uint32_t tileWidth = m_scene.GetSceneSettings().tileWidth;
    const uint32_t tileHeight = m_scene.GetSceneSettings().tileHeight;

    Texture2D* const targetTexure = m_scene.GetTargetTexture();
    if (targetTexure == nullptr)
    {
        std::cerr << "[Error] Target texture has not been set." << std::endl;
        return;
    }
    const uint32_t outputWidth = targetTexure->GetWidth();
    const uint32_t outputHeight = targetTexure->GetHeight();
    TileManager tileManager(outputWidth, outputHeight, tileWidth, tileHeight);

    m_scene.BuildAccel();

    std::mutex mtx;

    const uint32_t numThreads = m_scene.GetSceneSettings().numThreads > 0
                                  ? m_scene.GetSceneSettings().numThreads
                                  : std::thread::hardware_concurrency();

    std::cout << "Hardware Concurrency: " << std::thread::hardware_concurrency()
              << std::endl;
    std::cout << "Number of used threads: " << numThreads << std::endl;

    {
        uint32_t maxIdxTile = 0;
        ThreadPool<void> threadPool(numThreads);

        for (int idxTile = 0;
             idxTile < static_cast<int>(tileManager.GetNumTiles());
             idxTile++)
        {
            threadPool.Run([&, idxTile](size_t threadIndex) {
                const Tile tile = tileManager.GetTile();
                const auto pixelPos = tile.GetInitialPixel();

                const uint32_t i0 = pixelPos.first;
                const uint32_t j0 = pixelPos.second;

#ifdef _WIN32
                {
                    const int processerGroupID = threadIndex % 64;
                    GROUP_AFFINITY groupAffinity{};
                    if (GetNumaNodeProcessorMaskEx(processerGroupID,
                                                   &groupAffinity))
                    {
                        SetThreadGroupAffinity(
                          GetCurrentThread(), &groupAffinity, nullptr);
                    }
                }
#endif

                RandomSampler1D sampler1D(idxTile);
                RandomSampler2D sampler2D(idxTile, idxTile + 123456u);

                for (uint32_t j = j0; j < j0 + tile.GetHeight(); j++)
                {
                    for (uint32_t i = i0; i < i0 + tile.GetWidth(); i++)
                    {
                        pt.Render(
                          i, j, m_scene, targetTexure, sampler1D, sampler2D);
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    maxIdxTile =
                      std::max(maxIdxTile, static_cast<uint32_t>(idxTile));

                    const float ratio = 100.0f *
                                        static_cast<float>(maxIdxTile) /
                                        tileManager.GetNumTiles();

                    std::stringstream ss;
                    ss << "[PT: Rendering] " << (maxIdxTile + 1) << "/"
                       << tileManager.GetNumTiles() << "(" << std::fixed
                       << std::setprecision(2) << ratio << "%)"
                       << "\r" << std::flush;
                    std::cout << ss.str();
                }
            });
        }
    }
    Finalize();
}

void
Petrichor::SaveImage(const std::string& path)
{
    const auto& targetTex = m_scene.GetTargetTexture();
    targetTex->Save(path);
}

void
Petrichor::Finalize()
{
    // #DEBUG
    m_scene.GetEnvironment().m_debugTex.Save("sampling.hdr");

    const std::chrono::duration<float> totalTime =
      ClockType::now() - m_timeRenderingBegin;

    {
        RenderingResult renderingResult;
        renderingResult.totalSec = totalTime.count();
        m_onRenderingFinished(renderingResult);
    }

    std::cout << "[Finished]" << std::endl;
}

} // namespace Core
} // namespace Petrichor
