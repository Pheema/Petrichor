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
#include <filesystem>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Petrichor
{
namespace Core
{

//! #TODO: リソース管理用のクラスを作る？
struct TextureHandle
{
    uint32_t handle;
};

class Scene
{
private:
    //! メモリ再配置によって他から参照されているリソースへのポインタが無効になることを防ぐ。
    // #TODO: 設計がひどすぎるので、ハンドルかなにかで管理する形に変更する
    static constexpr size_t kNumMaxMaterials = 128;
    static constexpr size_t kNumMaxTextures = 128;

public:
    //! パスの種類
    struct AOVType
    {
        enum Value
        {
            Rendered,        //!< Rendered image
            DenoisingAlbedo, //!< Albedo for denoising
            DenoisingNormal, //!< Normal for denoising

            NumAOVTypes
        };
    };

public:
    Scene()
    {
        m_textures.reserve(kNumMaxTextures);
        m_materials.reserve(kNumMaxMaterials);
    }

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

    //! シーンにマテリアルを登録
    void
    RegisterMaterial(std::string_view materialName,
                     std::unique_ptr<const MaterialBase> material)
    {
        m_materials[std::string(materialName)] = std::move(material);
    }

    //! シーンに登録されたマテリアルを取得
    const MaterialBase*
    GetMaterial(std::string_view materialName)
    {
        return m_materials[std::string(materialName)].get();
    }

    //! シーンにテクスチャを登録
    TextureHandle
    RegisterTexture(const Texture2D& texture2D)
    {
        const TextureHandle textureHandle = [&]() {
            TextureHandle textureHandle_;
            textureHandle_.handle =
              static_cast<decltype(TextureHandle::handle)>(m_textures.size());
            return textureHandle_;
        }();

        m_textures.emplace_back(texture2D);
        return textureHandle;
    }

    //! シーンに登録してあるテクスチャを取得
    const Texture2D&
    GetTexture(const TextureHandle& textureHandle)
    {
        return m_textures[textureHandle.handle];
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
    SetTargetTexture(AOVType::Value aovType, Texture2D* targetTex)
    {
        m_targetTextures[aovType] = targetTex;
    }

    // レンダリング先のテクスチャを取得
    Texture2D*
    GetTargetTexture(AOVType::Value aovType) const
    {
        return m_targetTextures[aovType];
    }

    //! シーン設定を読み込む
    void
    LoadRenderSetting(const std::filesystem::path& path);

    //! モデルファイルを読み込む
    // #TODO: ひとまず用意。あとでシーンファイル用意したら消すかも。
    void
    LoadModel(const std::filesystem::path& path);

    //
    void
    LoadModel(const std::filesystem::path& path, std::string_view materialName);

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
    // #TODO:
    // 時間ないのでひとまずこのまま。後でマテリアルの実態には、局所性を持たせたい。
    std::unordered_map<std::string, std::unique_ptr<const MaterialBase>>
      m_materials;

    //! レンダリングで使用するテクスチャ
    std::vector<Texture2D> m_textures{};

    //! 環境マップ
    Environment m_environment;

    //! メインカメラ
    std::unique_ptr<Camera> m_mainCamera = nullptr;

    //! レンダリング先のテクスチャ
    std::array<Texture2D*, AOVType::NumAOVTypes> m_targetTextures = {};

    //! レンダリング設定
    RenderSetting m_renderSetting;
};

} // namespace Core
} // namespace Petrichor
