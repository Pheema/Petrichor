#pragma once

#include <vector>

#include <Core/Camera.h>
#include <Core/Environment.h>
#include <Core/Geometry/GeometryBase.h>
#include <Core/Geometry/Mesh.h>


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
    AppendGeometry(const GeometryBase* geometry);

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
    GetGeometries() const;

    // ライトのリストを取得
    const std::vector<const GeometryBase*>&
    GetLights() const
    {
        return m_lights;
    }

    // メインカメラを取得
    const Camera*
    GetMainCamera() const;

    // メインカメラを登録
    void
    SetMainCamera(const Camera& camera);

    // Environmentを取得
    Environment&
    GetEnvironment() const;

    void
    SetTargetTexture(Texture2D* targetTex)
    {
        m_targetTex = targetTex;
    }

    const Texture2D*
    GetTargetTexture() const
    {
        return m_targetTex;
    }

private:
    // シーンに登録されたオブジェクト
    std::vector<const GeometryBase*> m_geometries;

    // シーンに登録された
    std::vector<const GeometryBase*> m_lights;

    mutable Environment m_environment;

    // メインカメラ
    const Camera* m_mainCamera;

    // レンダリング先のテクスチャ
    Texture2D* m_targetTex = nullptr;

};

#pragma region Inline functions

    inline void
    Scene::AppendGeometry(const GeometryBase* geometry)
    {
        m_geometries.emplace_back(geometry);
    }

    inline const std::vector<const GeometryBase*>&
    Scene::GetGeometries() const
    {
        return m_geometries;
    }

    inline const Camera*
    Scene::GetMainCamera() const
    {
        return m_mainCamera;
    }

    inline void
    Scene::SetMainCamera(const Camera& camera)
    {
        m_mainCamera = &camera;
    }

    inline Environment&
    Scene::GetEnvironment() const
    {
        return m_environment;
    }

#pragma endregion

}   // namespace Core
}   // namespace Petrichor
