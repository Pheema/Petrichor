#pragma once

#include "Core/Accel/AccelBase.h"
#include "Core/Accel/BVH.h"
#include "Core/Accel/BruteForce.h"
#include "Core/Camera.h"
#include "Core/Environment.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Geometry/Mesh.h"
#include "Core/SceneSettings.h"
#include <optional>
#include <vector>

namespace Petrichor
{
namespace Core
{

class Scene
{
public:
    Scene() = default;

    // シーンにジオメトリを追加
    void
    AppendGeometry(const GeometryBase* geometry)
    {
        m_geometries.emplace_back(geometry);
    }

    // シーンに頂点を追加
    void
    AppendMesh(const Mesh& mesh)
    {
        for (const auto& mesh : mesh.GetTriangles())
        {
            AppendGeometry(&mesh);
        }
    }

    // シーンにメッシュライトを登録
    void
    AppendLightMesh(const Mesh& mesh)
    {
        for (const auto& mesh : mesh.GetTriangles())
        {
            AppendGeometry(&mesh);
            AppendLight(&mesh);
        }
    }

    // シーンにライトを登録
    void
    AppendLight(const GeometryBase* geometry)
    {
        m_lights.emplace_back(geometry);
    }

    // ジオメトリのリストを取得
    const std::vector<const GeometryBase*>&
    GetGeometries() const
    {
        return m_geometries;
    }

    // ライトのリストを取得
    const std::vector<const GeometryBase*>&
    GetLights() const
    {
        return m_lights;
    }

    // メインカメラを取得

    const Camera*
    GetMainCamera() const
    {
        return m_mainCamera;
    }

    // メインカメラを登録
    void
    SetMainCamera(const Camera& camera)
    {
        m_mainCamera = &camera;
    }

    // Environmentを取得
    Environment&
    GetEnvironment() const
    {
        return m_environment;
    }

    // レンダリング先のテクスチャを設定
    void
    SetTargetTexture(Texture2D* targetTex)
    {
        m_targetTex = targetTex;
    }

    // レンダリング先のテクスチャを取得
    Texture2D*
    GetTargetTexture() const
    {
        return m_targetTex;
    }

    void
    BuildAccel()
    {
        ASSERT(m_accel != nullptr &&
               "Acceleration structure has not been assigned.");
        m_accel = std::make_unique<BruteForce>();
        m_accel->Build(*this);
    }

    std::optional<HitInfo>
    Intersect(const Ray& ray) const
    {
        ASSERT(m_accel != nullptr);
        return m_accel->Intersect(ray);
    }

    std::optional<HitInfo>
    Intersect(const Ray& ray, float distMin) const
    {
        ASSERT(m_accel != nullptr);
        return m_accel->Intersect(ray, distMin);
    }

    //! シーン設定を読み込む
    void
    LoadSceneSettings();

    const SceneSettings&
    GetSceneSettings() const
    {
        return m_sceneSetting;
    }

private:
    // シーンに登録されたオブジェクト
    std::vector<const GeometryBase*> m_geometries;

    //シーンに登録されたライト
    std::vector<const GeometryBase*> m_lights;

    mutable Environment m_environment;

    // メインカメラ
    const Camera* m_mainCamera = nullptr;

    // 高速化構造
    std::unique_ptr<AccelBase> m_accel = nullptr;

    // レンダリング先のテクスチャ
    Texture2D* m_targetTex = nullptr;

    // シーン設定
    SceneSettings m_sceneSetting;
};

} // namespace Core
} // namespace Petrichor
