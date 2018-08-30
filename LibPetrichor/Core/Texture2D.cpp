#include "Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Petrichor
{
namespace Core
{

namespace
{

float
ACESFilm(float x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return std::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

Color3f
ACESFilm(const Color3f& color)
{
    return { ACESFilm(color.x), ACESFilm(color.y), ACESFilm(color.z) };
}

Color3f
Clamp(const Color3f& color, float min, float max)
{
    return { std::clamp(color.x, min, max),
             std::clamp(color.y, min, max),
             std::clamp(color.z, min, max) };
}

float
ApplyGamma(float value)
{
    return std::pow(value, 2.2f);
}

Color3f
ApplyGamma(const Color3f& color)
{
    return { ApplyGamma(color.x), ApplyGamma(color.y), ApplyGamma(color.z) };
}

Color3f
ApplyToneMapping(const Math::Vector3f& color)
{
    return { ACESFilm(color.x), ACESFilm(color.y), ACESFilm(color.z) };
}

float
ApplyDegamma(float value)
{
    value = std::clamp(value, 0.0f, 1.0f);
    return std::pow(value, 1 / 2.2f);
}

Color3f
ApplyDegamma(const Color3f& color)
{
    return { ApplyDegamma(color.x),
             ApplyDegamma(color.y),
             ApplyDegamma(color.z) };
}

} // namespace

Texture2D::Texture2D()
  : Texture2D(0, 0)
{
}

Texture2D::Texture2D(int width, int height)
  : m_width(width)
  , m_height(height)
{
    Clear();
}

bool
Texture2D::Load(const std::filesystem::path& path,
                TextureColorType textureColorType)
{
    if (std::filesystem::exists(path))
    {
        std::cerr << "[Error] Texture not found. (" << path << ")" << std::endl;
        return false;
    }

    // #TODO: とりあえずif
    if (path.extension() == ".png")
    {
        const unsigned char* data = stbi_load(path.string().c_str(),
                                              &m_width,
                                              &m_height,
                                              nullptr,
                                              kNumChannelsInPixelRGB);

        if (data == nullptr)
        {
            std::cerr << "[Error] Could not read png file. (" << path << ")"
                      << std::endl;
            return false;
        }

        const int numPixels = m_width * m_height;

        m_pixels.clear();
        m_pixels.shrink_to_fit();
        m_pixels.reserve(numPixels);
        for (size_t i = 0; i < numPixels; i++)
        {
            // 8Bit/Channel前提
            Color3f color{
                static_cast<float>(data[kNumChannelsInPixelRGB * i]),
                static_cast<float>(data[kNumChannelsInPixelRGB * i + 1]),
                static_cast<float>(data[kNumChannelsInPixelRGB * i + 2])
            };
            color /= static_cast<float>(kColorDepth8Bit);
            m_pixels.emplace_back(color);
        }

        // カラー用テクスチャの場合はガンマ補正
        if (textureColorType == TextureColorType::Color)
        {
            for (auto& pixel : m_pixels)
            {
                pixel = ApplyGamma(pixel);
            }
        }

        stbi_image_free(const_cast<unsigned char*>(data));
        return true;
    }

    if (path.extension() == ".hdr")
    {
        stbi_hdr_to_ldr_scale(1.0f);
        const float* const data =
          stbi_loadf(path.string().c_str(), &m_width, &m_height, nullptr, 0);

        if (data == nullptr)
        {
            std::cerr << "[Error] Could not read hdr file.(" << path << ")"
                      << std::endl;
            return false;
        }

        const int numPixels = m_width * m_height;

        m_pixels.clear();
        m_pixels.shrink_to_fit();
        m_pixels.reserve(numPixels);
        for (size_t i = 0; i < numPixels; i++)
        {
            Color3f color{ data[kNumChannelsInPixelHDR * i],
                           data[kNumChannelsInPixelHDR * i + 1],
                           data[kNumChannelsInPixelHDR * i + 2] };
            m_pixels.emplace_back(color);
        }

        stbi_image_free(const_cast<float*>(data));
        return true;
    }

    std::cerr << "[Error] Unsupported texture format.(" << path << ")"
              << std::endl;
    return false;
}

void
Texture2D::Save(std::filesystem::path path) const
{
    if (path.extension() == ".png")
    {
        std::vector<uint8_t> outPixels;
        outPixels.reserve(kNumChannelsInPixelRGB * m_width * m_height);

        for (const auto& pixel : m_pixels)
        {
            const Color3f toneMapped = ACESFilm(pixel);
            const Color3f degammaColor =
              ApplyDegamma(Clamp(toneMapped, 0.0f, 1.0f));

            const auto r =
              static_cast<uint8_t>((kColorDepth8Bit - kEps) * degammaColor.x);
            const auto g =
              static_cast<uint8_t>((kColorDepth8Bit - kEps) * degammaColor.y);
            const auto b =
              static_cast<uint8_t>((kColorDepth8Bit - kEps) * degammaColor.z);

            outPixels.emplace_back(r);
            outPixels.emplace_back(g);
            outPixels.emplace_back(b);
        }

        stbi_write_png(path.string().c_str(),
                       m_width,
                       m_height,
                       kNumChannelsInPixelRGB,
                       &outPixels[0],
                       m_width * kNumChannelsInPixelRGB);
        return;
    }

    if (path.extension() == ".hdr")
    {
        std::vector<float> outPixels;
        outPixels.reserve(kNumChannelsInPixelHDR * m_width * m_height);

        for (const auto& pixel : m_pixels)
        {
            outPixels.emplace_back(pixel.x);
            outPixels.emplace_back(pixel.y);
            outPixels.emplace_back(pixel.z);
        }

        stbi_write_hdr(path.string().c_str(),
                       m_width,
                       m_height,
                       kNumChannelsInPixelHDR,
                       &outPixels[0]);
        return;
    }

    throw std::runtime_error("Unsupported image type.");
}

void
Texture2D::Clear(const Color3f& color)
{
    m_pixels.resize(m_width * m_height, color);
}

Color3f
Texture2D::GetPixel(int i, int j) const
{
    if (!IsValid())
    {
        return Color3f(1.0f, 0.0f, 1.0f);
    }

    ASSERT(0 <= i && i < m_width);
    ASSERT(0 <= j && j < m_height);
    return m_pixels[m_width * j + i];
}

Color3f
Texture2D::GetPixel(float x,
                    float y,
                    InterplationTypes interpoplationType) const
{
    switch (interpoplationType)
    {
    case InterplationTypes::Point:
    {
        int i = static_cast<int>(x);
        int j = static_cast<int>(y);
        return GetPixel(i, j);
    }

    case InterplationTypes::Bilinear:
    {
        int ix0 = static_cast<int>(x);
        int ix1 = static_cast<int>(x + 1.0f);
        int iy0 = static_cast<int>(y);
        int iy1 = static_cast<int>(y + 1.0f);

        // 重み係数
        float wx0 = x - ix0;
        float wy0 = y - iy0;
        float wx1 = ix1 - x;
        float wy1 = iy1 - y;

        // 4方のピクセル色
        ix0 = std::clamp(ix0, 0, GetWidth() - 1);
        ix1 = std::clamp(ix1, 0, GetWidth() - 1);
        iy0 = std::clamp(iy0, 0, GetHeight() - 1);
        iy1 = std::clamp(iy1, 0, GetHeight() - 1);

        auto c00 = GetPixel(ix0, iy0);
        auto c10 = GetPixel(ix1, iy0);
        auto c01 = GetPixel(ix0, iy1);
        auto c11 = GetPixel(ix1, iy1);

        auto cm0 = wx1 * c00 + wx0 * c10;
        auto cm1 = wx1 * c01 + wx0 * c11;

        auto c = wy1 * cm0 + wy0 * cm1;

        return c;
    }

    default:
    {
        ASSERT(false && "Invalid Interpolation types");
        return GetPixel(0, 0);
    }
    }
}

Color3f
Texture2D::GetPixelByUV(float u,
                        float v,
                        InterplationTypes interpoplationType) const
{
    u = Math::Mod(u, 1.0f);
    v = Math::Mod(v, 1.0f);
    return GetPixel(u * m_width, v * m_height, interpoplationType);
}

void
Texture2D::SetPixel(int i, int j, const Color3f& color)
{
    ASSERT(0 <= i && i < m_width);
    ASSERT(0 <= j && j < m_height);
    m_pixels[m_width * j + i] = color;
}

} // namespace Core
} // namespace Petrichor
