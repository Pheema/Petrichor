#pragma once

#include "Core/Accel/AccelBase.h"
#include "Core/Accel/BinnedSAHBVH.h"
#include "Core/Accel/BruteForce.h"
#include "Core/Camera.h"
#include "Core/Environment.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Material/MaterialBase.h"
#include "Core/RenderSetting.h"

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
    //! パスの種類
    struct RenderPassType
    {
        enum Value
        {
            Rendered,
            Normal,

            RenderPassTypeMax
        };
    };

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

    // メインカメラを参照
    const Camera*
    GetMainCamera() const
    {
        return m_mainCamera.get();
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
    // #TODO: そもそも外部からカメラを登録するのはどうなの…。
    void
    SetMainCamera(std::unique_ptr<Camera> camera)
    {
        m_mainCamera = std::move(camera);
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
    SetTargetTexture(RenderPassType::Value renderPassType, Texture2D* targetTex)
    {
        m_targetTextures[renderPassType] = targetTex;
    }

    // レンダリング先のテクスチャを取得
    Texture2D*
    GetTargetTexture(RenderPassType::Value renderPassType) const
    {
        return m_targetTextures[renderPassType];
    }

    //! シーン設定を読み込む
    void
    LoadRenderSetting(const std::filesystem::path& path);

    //! モデルファイルを読み込む
    // #TODO: ひとまず用意。あとでシーンファイル用意したら消すかも。
    void
    LoadModel(const std::filesystem::path& path);

    //! シーンファイルを元にして各モデルを読む
    void
    LoadAssets(const std::filesystem::path path);

    //! 環境マップを取得
    const RenderSetting&
    GetRenderSetting() const
    {
        return m_renderSetting;
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
    std::unique_ptr<Camera> m_mainCamera = nullptr;

    //! レンダリング先のテクスチャ
    std::array<Texture2D*, RenderPassType::RenderPassTypeMax>
      m_targetTextures = {};

    //! レンダリング設定
    RenderSetting m_renderSetting;
};

} // namespace Core
} // namespace Petrichor
