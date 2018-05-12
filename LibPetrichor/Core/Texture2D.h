#pragma once

#include <string>
#include <vector>
#include <Core/Color3f.h>

namespace Petrichor
{
namespace Core
{
    class Texture2D
    {
    public:
#pragma region Enums

        enum class ImageTypes
        {
            Jpg,
            Png
        };

        enum class TextureFormat
        {
            RGB,
            RGBA    // TODO: 未実装
        };

        enum class InterplationTypes
        {
            Point,
            Bilinear
        };

#pragma endregion

        Texture2D();

        Texture2D(int width, int height);

        // 画像を読み込む
        void
        Load(std::string path);

        //// 画像を書き出し
        void
        Save(std::string path, ImageTypes imageType = ImageTypes::Png) const;

        //// 画像をクリア
        void
        Clear(const Color3f& color = Color3f::Zero());

        // 指定ピクセルの色を取得
        Color3f
        GetPixel(int i, int j) const;

        // 指定ピクセルの色を取得(補間付き)
        Color3f
        GetPixel(float x, float y, InterplationTypes interpoplationType = InterplationTypes::Bilinear) const;

        Color3f
        GetPixelByUV(float u, float v, InterplationTypes interpoplationType = InterplationTypes::Bilinear) const;

        //// 指定ピクセルに色をセット
        void
        SetPixel(int i, int j, const Color3f& color);

        // 画像の幅を取得
        int
        GetWidth() const;
        
        // 画像の高さを取得
        int
        GetHeight() const;

    private:
        // TODO: RGBAに対応させる（現在はRGB）
        static const int kNumChannelsInPixel = 3;

        int m_width;                            // 画像の横幅[px]
        int m_height;                           // 画像の縦幅[px]
        InterplationTypes m_interplationType;   // 画像の補間タイプ
        std::vector<Color3f> m_pixels;          // 画素値の配列

    };
}   // namespace Core
}   // namespace Petrichor
