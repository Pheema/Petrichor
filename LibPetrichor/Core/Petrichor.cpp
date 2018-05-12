#include "Petrichor.h"

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
#include <fstream>

namespace Petrichor
{
namespace Core
{

void
Petrichor::Initialize()
{
    m_timeBegin = std::chrono::high_resolution_clock::now();

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

    // 環境マップの設定
    m_scene.GetEnvironment().Load("Resource/balcony_2k.png");
    m_scene.GetEnvironment().SetBaseColor(0.0f * Color3f::One());
}

void
Petrichor::Render()
{
    // uint32_t height = 1080;
    uint32_t height = 540;
    auto targetTex  = new Texture2D(height * 3 / 2, height);
    m_scene.SetTargetTexture(targetTex);

    PathTracing pt;
    pt.Render(m_scene, targetTex);
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
    const auto& targetTex = m_scene.GetTargetTexture();
    targetTex->Save("final.png");

    auto timeEnd     = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      timeEnd - m_timeBegin);
    std::cout << "Elapsed time: " << elapsedTime.count() << " msec"
              << std::endl;

    std::ofstream of("log.txt");
    of << elapsedTime.count() << "[msec]" << std::endl;
    of.close();

    std::cout << "[Finished]" << std::endl;
}

} // namespace Core
} // namespace Petrichor
