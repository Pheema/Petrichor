#include "TestScene.h"

namespace Petrichor
{
namespace Core
{

void
LoadCornellBoxScene(Scene* scene)
{
    if (scene == nullptr)
    {
        return;
    }

    scene->LoadRenderSetting("Resource/SampleScene/CornellBox/settings.json");

    const auto* matLambertRed = new Lambert(Color3f(1.0f, 0, 0));
    const auto* matLambertGreen = new Lambert(Color3f(0, 1.0f, 0));
    const auto* matLamberWhite = new Lambert(Color3f::One());
    const auto* matEmissionWhite = new Emission(Color3f::One());

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
                   matLamberWhite,
                   ShadingTypes::Flat);

    auto const ceilLight = new Mesh();
    ceilLight->Load("Resource/SampleScene/CornellBox/CeilLight.obj",
                    matEmissionWhite,
                    ShadingTypes::Flat);

    scene->AppendMesh(*leftWall);
    scene->AppendMesh(*rightWall);
    scene->AppendMesh(*whiteWall);
    scene->AppendMesh(*whiteBox);
    scene->AppendLightMesh(*ceilLight);

    auto const camera =
      new Camera(Math::Vector3f(0, -6.0f, 0), Math::Vector3f::UnitY());

    camera->FocusTo(Math::Vector3f::Zero());

    scene->SetMainCamera(*camera);

    // レンダリング先を指定
    auto targetTex = new Texture2D(scene->GetRenderSetting().outputWidth,
                                   scene->GetRenderSetting().outputHeight);
    scene->SetTargetTexture(targetTex);
}

} // namespace Core
} // namespace Petrichor
