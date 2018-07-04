#include "Petrichor.h"

#include "Core/Accel/BVH.h"
#include "Core/Camera.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/Triangle.h"
#include "Core/Geometry/Vertex.h"
#include "Core/Integrator/PathTracing.h"
#include "Core/Material/Emission.h"
#include "Core/Material/GGX.h"
#include "Core/Material/Lambert.h"
#include "Core/Material/MatMix.h"
#include "Core/Sampler/RandomSampler1D.h"
#include "Core/Sampler/RandomSampler2D.h"
#include "Core/TileManager.h"
#include "Random/XorShift.h"
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace Petrichor
{
namespace Core
{

void
Petrichor::Initialize()
{

#if 0

    // シーンの定義
    auto sphere = new Sphere(Math::Vector3f(-1.5f, 0.0f, 1.0f), 1.0f);

    float r    = 1000.0f;
    auto floor = new Sphere(-Math::Vector3f::UnitZ() * r, r);

    auto sphereLight = new Sphere(Math::Vector3f(2.5f, -2.0f, 1.0f), 0.5f);

    // マテリアルを適用
    // TODO: メモリリーク

    // ---- floorMat ----
    MaterialBase* matLambertFloor = new Lambert(0.95f * Color3f::One());
    MaterialBase* matGGXFloor     = new GGX(0.05f * Color3f::One(), 0.05f);
    MaterialBase* matMixFloor = new MatMix(matLambertFloor, matGGXFloor, 0.3f);

    // ---- matBody ----
    MaterialBase* matLambertBody = new Lambert(0.01f * Color3f::One());

    MaterialBase* matGGXBodySmooth = new GGX(0.05f * Color3f::One(), 0.1f);
    MaterialBase* matBodySmooth =
      new MatMix(matLambertBody, matGGXBodySmooth, 0.2f);

    MaterialBase* matGGXBodyRough = new GGX(0.05f * Color3f::One(), 0.3f);
    MaterialBase* matBodyRough =
      new MatMix(matLambertBody, matGGXBodyRough, 0.2f);

    MaterialBase* matArrayBody[] = { matGGXBodySmooth, matGGXBodyRough };

    // ---- matCover ----
    MaterialBase* matLambertCover = new Lambert(1.5f * Color3f::One());
    MaterialBase* matGGXCover     = new GGX(0.05f * Color3f::One(), 0.31f);
    MaterialBase* matMixCover = new MatMix(matLambertCover, matGGXCover, 0.1f);
    auto* texCover            = new Texture2D();
    texCover->Load("Resource/fabric_01_diffuse.jpg");
    static_cast<Lambert*>(matLambertCover)->SetTexAlbedo(texCover);

    // ---- matCode ----
    MaterialBase* matLambertCode = new Lambert(0.03f * Color3f::One());
    MaterialBase* matGGXCode     = new GGX(0.05f * Color3f::One(), 0.1f);
    MaterialBase* matMixCode     = new MatMix(matLambertCode, matGGXCode, 0.1f);

    // ---- panelLight ----
    MaterialBase* matLightLU = new Emission(5.0f * Color3f::One());
    MaterialBase* matLightR  = new Emission(2.0f * Color3f::One());

    auto meshFloor   = new Mesh();
    auto meshBody    = new Mesh();
    auto meshCode    = new Mesh();
    auto meshCover   = new Mesh();
    auto meshLightLU = new Mesh();
    auto meshLightR  = new Mesh();

    meshBody->Load(
      "Resource/SpeakerDiv3.obj", matArrayBody, 2, ShadingTypes::Smooth);
    meshCover->Load(
      "Resource/CoverMesh.obj", &matMixCover, 1, ShadingTypes::Smooth);
    meshCode->Load("Resource/Code.obj", &matMixCode, 1, ShadingTypes::Smooth);
    meshLightLU->Load(
      "Resource/PanelLightLU.obj", &matLightLU, 1, ShadingTypes::Flat);
    meshLightR->Load(
      "Resource/PanelLightR.obj", &matLightR, 1, ShadingTypes::Flat);

    meshFloor->Load("Resource/Floor.obj", &matMixFloor, 1, ShadingTypes::Flat);

    // sphere->SetMaterial(matMixFloor);
    floor->SetMaterial(matMixFloor);

    m_scene.AppendMesh(*meshBody);
    m_scene.AppendMesh(*meshCode);
    m_scene.AppendMesh(*meshCover);
    m_scene.AppendLightMesh(*meshLightLU);
    m_scene.AppendLightMesh(*meshLightR);

    // m_scene.AppendMesh(*meshFloor);

    // m_scene.AppendGeometry(sphere);
    m_scene.AppendGeometry(floor);
    // m_scene.AppendGeometry(sphereLight);
    // m_scene.AppendLight(*sphereLight);

    // カメラをセット
    auto camera = new Camera(Math::Vector3f(-7.53405f, -8.51452f, 5.22279f),
                             Math::Vector3f::UnitY());

    camera->LookAt(Math::Vector3f(-0.99248f, 0.14638f, 1.33148f));
    // TODO: メモリリーク

    // camera->SetFNumber(1.8f);
    // camera->SetLens(10e-3f);
    camera->SetLens(80e-3f);
    camera->FocusTo(sphere->o);
    m_scene.SetMainCamera(*camera);

#else

    MaterialBase* matLambertRed    = new Lambert(Color3f(1.0f, 0, 0));
    MaterialBase* matLambertGreen  = new Lambert(Color3f(0, 1.0f, 0));
    MaterialBase* matLamberWhite   = new Lambert(Color3f::One());
    MaterialBase* matEmissionWhite = new Emission(3.0f * Color3f::One());

    Mesh* const leftWall = new Mesh();
    leftWall->Load("Resource/SampleScene/CornellBox/LeftWall.obj",
                   &matLambertRed,
                   1,
                   ShadingTypes::Flat);

    Mesh* const rightWall = new Mesh();
    rightWall->Load("Resource/SampleScene/CornellBox/RightWall.obj",
                    &matLambertGreen,
                    1,
                    ShadingTypes::Flat);

    Mesh* const whiteWall = new Mesh();
    whiteWall->Load("Resource/SampleScene/CornellBox/WhiteWall.obj",
                    &matLamberWhite,
                    1,
                    ShadingTypes::Flat);

    Mesh* const whiteBox = new Mesh();
    whiteBox->Load("Resource/SampleScene/CornellBox/WhiteBox.obj",
                   &matLamberWhite,
                   1,
                   ShadingTypes::Flat);

    Mesh* const ceilLight = new Mesh();
    ceilLight->Load("Resource/SampleScene/CornellBox/CeilLight.obj",
                    &matEmissionWhite,
                    1,
                    ShadingTypes::Flat);

    m_scene.LoadSceneSettings();

    m_scene.AppendMesh(*leftWall);
    m_scene.AppendMesh(*rightWall);
    m_scene.AppendMesh(*whiteWall);
    m_scene.AppendMesh(*whiteBox);
    m_scene.AppendLightMesh(*ceilLight);

    Camera* const camera =
      new Camera(Math::Vector3f(0, -6.0f, 0), Math::Vector3f::UnitY());

    camera->FocusTo(Math::Vector3f::Zero());

    m_scene.SetMainCamera(*camera);

#endif

    // 環境マップの設定
    // m_scene.GetEnvironment().Load("Resource/balcony_2k.png");
    // m_scene.GetEnvironment().SetBaseColor(0.1f * Color3f::One());

    // レンダリング先を指定
    auto targetTex = new Texture2D(m_scene.GetSceneSettings().outputWidth,
                                   m_scene.GetSceneSettings().outputHeight);
    m_scene.SetTargetTexture(targetTex);
}

void
Petrichor::Render()
{
    PathTracing pt;

    const uint32_t tileWidth  = m_scene.GetSceneSettings().tileWidth;
    const uint32_t tileHeight = m_scene.GetSceneSettings().tileHeight;

    Texture2D* const targetTexure = m_scene.GetTargetTexture();
    if (targetTexure == nullptr)
    {
        std::cerr << "[Error] Target texture has not been set." << std::endl;
        return;
    }
    const uint32_t outputWidth  = targetTexure->GetWidth();
    const uint32_t outputHeight = targetTexure->GetHeight();
    TileManager tileManager(outputWidth, outputHeight, tileWidth, tileHeight);

    m_scene.BuildAccel();

    std::mutex mtx;
    uint32_t maxIdxTile = 0;
#pragma omp parallel for num_threads(4) schedule(dynamic)
    for (int idxTile = 0; idxTile < static_cast<int>(tileManager.GetNumTiles());
         idxTile++)
    {
        const Tile tile     = tileManager.GetTile();
        const auto pixelPos = tile.GetInitialPixel();

        const uint32_t i0 = pixelPos.first;
        const uint32_t j0 = pixelPos.second;

        RandomSampler1D sampler1D(idxTile);
        RandomSampler2D sampler2D(idxTile);

        for (uint32_t j = j0; j < j0 + tile.GetHeight(); j++)
        {
            for (uint32_t i = i0; i < i0 + tile.GetWidth(); i++)
            {
                pt.Render(i, j, m_scene, targetTexure, sampler1D, sampler2D);
            }
        }

        {
            mtx.lock();
            maxIdxTile = std::max(maxIdxTile, static_cast<uint32_t>(idxTile));

            const float ratio = 100.0f * static_cast<float>(maxIdxTile) /
                                tileManager.GetNumTiles();

            std::stringstream ss;
            ss << "[PT: Rendering] " << (maxIdxTile + 1) << "/"
               << tileManager.GetNumTiles() << "(" << std::fixed
               << std::setprecision(2) << ratio << "%)"
               << "\r" << std::flush;
            std::cout << ss.str();
            mtx.unlock();
        }
    }
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
    std::cout << "[Finished]" << std::endl;
}

} // namespace Core
} // namespace Petrichor
