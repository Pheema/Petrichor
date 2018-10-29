#pragma once

#include "Core/Accel/AccelBase.h"
#include "Core/Accel/BinnedSAHBVH.h"
#include "Core/Accel/BruteForce.h"
#include "Core/Camera.h"
#include "Core/Environment.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Material/MaterialBase.h"
#include "Core/SceneSettings.h"
#include <any>
#include <filesystem>
#include <optional>
#include <vector>

namespace Petrichor
{
namespace Core
{

class Scene
{
    // #TODO: マテリアルの個数をファイルから読みこんで動的に変更する
    static constexpr int kMaxNumMaterials = 128;

public:
    Scene() { m_materials.reserve(kMaxNumMaterials); };

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

    //! シーン内で使用しているマテリアルを登録
    template<class T>
    T*
    AppendMaterial(T material)
    {
        static_assert(std::is_base_of<MaterialBase, T>::value);

        if (m_materials.size() >= kMaxNumMaterials)
        {
            throw std::runtime_error("Too many materials.");
        }
        return std::any_cast<T>(&m_materials.emplace_back(material));
    }

    // メインカメラを登録
    void
    SetMainCamera(const Camera& camera)
    {
        m_mainCamera = &camera;
    }

    // Environmentを取得
    const Environment&
    GetEnvironment() const
    {
        return m_environment;
    }

    //! 環境マップを設定
    void
    SetEnvironment(const Environment& environment)
    {
        m_environment = environment;
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

    //! シーン設定を読み込む
    void
    LoadSceneSettings(const std::filesystem::path& path);

    //! 環境マップを取得
    const SceneSettings&
    GetSceneSettings() const
    {
        return m_sceneSetting;
    }

private:
    //! シーンに登録されたオブジェクト
    std::vector<const GeometryBase*> m_geometries;

    //! シーンに登録されたライト
    std::vector<const GeometryBase*> m_lights;

    //! シーンで使用するマテリアル
    std::vector<std::any> m_materials;

    //! 環境マップ
    Environment m_environment;

    //! メインカメラ
    const Camera* m_mainCamera = nullptr;

    //! レンダリング先のテクスチャ
    Texture2D* m_targetTex = nullptr;

    //! シーン設定
    SceneSettings m_sceneSetting;
};

} // namespace Core
} // namespace Petrichor
