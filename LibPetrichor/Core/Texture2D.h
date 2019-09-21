#pragma once

#include "Core/Color3f.h"
#include <filesystem>
#include <string>
#include <vector>

namespace Petrichor
{
namespace Core
{
class Texture2D
{
public:
    enum class ImageTypes
    {
        Jpg,
        Png
    };

    enum class TextureFormat
    {
        RGB,
        RGBA // TODO: 未実装
    };

    enum class InterplationTypes
    {
        Point,
        Bilinear
    };

    enum class TextureColorType
    {
        Color,   //!< 読み込むときにガンマ補正をする
        NonColor //!< 読み込むときにガンマ補正をかけない
    };

    Texture2D();

    Texture2D(int width, int height);

    //! 画像を読み込む
    //! @return 読み込みが成功したか
    bool
    Load(const std::filesystem::path& path, TextureColorType textureColorType);

    //// 画像を書き出し
    void
    Save(const std::filesystem::path& path) const;

    //// 画像をクリア
    void
    Clear(const Color3f& color = Color3f::Zero());

    // 指定ピクセルの色を取得
    Color3f
    GetPixel(int i, int j) const;

    // 指定ピクセルの色を取得(補間付き)
    Color3f
    GetPixel(
      float x,
      float y,
      InterplationTypes interpoplationType = InterplationTypes::Bilinear) const;

    Color3f
    GetPixelByUV(
      float u,
      float v,
      InterplationTypes interpoplationType = InterplationTypes::Bilinear) const;

    //// 指定ピクセルに色をセット
    void
    SetPixel(int i, int j, const Color3f& color);

    // 画像の幅を取得

    int
    GetWidth() const
    {
        return m_width;
    }

    // 画像の高さを取得

    int
    GetHeight() const
    {
        return m_height;
    }

    bool
    IsValid() const
    {
        return m_isLoaded && (GetWidth() > 0) && (GetHeight() > 0);
    }

    //! 再配置が起きる可能性があるのでポインタの保持には注意
    const float*
    GetRawDataPtr() const
    {
        return &(m_pixels[0].x);
    }

    float*
    GetRawDataPtr()
    {
        return const_cast<float*>(
          static_cast<const Texture2D*>(this)->GetRawDataPtr());
    }

private:
    // TODO: RGBAに対応させる（現在はRGB）
    static constexpr int kNumChannelsInPixelRGB = 3;
    static constexpr int kNumChannelsInPixelHDR = 3;
    static constexpr int kColorDepth8Bit = 255;

    int m_width = 0;  //!< 画像の横幅[px]
    int m_height = 0; //!< 画像の縦幅[px]

    InterplationTypes m_interplationType =
      InterplationTypes::Bilinear; //<! 画像の補間タイプ

    std::vector<Color3f> m_pixels; //!< 画素値の配列

    bool m_isLoaded = false; //!< 読み込みが成功しているか
};

} // namespace Core
} // namespace Petrichor
