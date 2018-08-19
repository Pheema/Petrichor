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

float
ApplyGamma(unsigned char val)
{
    const float kDepthMax = 255.0f;
    return std::pow(val / kDepthMax, 2.2f);
}

uint8_t
ApplyDegamma(float val)
{
    // val = ACESFilm(val);
    val = std::pow(val, 1 / 2.2f);
    val = std::clamp(val, 0.0f, 1.0f);
    constexpr float kDepthMax = 255.9999f;
    return static_cast<uint8_t>(kDepthMax * val);
}
} // namespace

Texture2D::Texture2D()
  : m_width(0)
  , m_height(0)
  , m_interplationType(InterplationTypes::Point)
{
    Clear();
}

Texture2D::Texture2D(int width, int height)
  : m_width(width)
  , m_height(height)
  , m_interplationType(InterplationTypes::Point)
{
    Clear();
}

void
Texture2D::Load(std::string path)
{
    unsigned char* data = stbi_load(
      path.c_str(), &m_width, &m_height, nullptr, kNumChannelsInPixel);

    if (data == nullptr)
    {
        std::cerr << "[Texture Loading Error] " << path << std::endl;
        return;
    }

    const int numPixels = m_width * m_height;
    for (size_t i = 0; i < numPixels; i++)
    {
        // 8Bit/Channel前提
        Color3f color;
        color.x = ApplyGamma(data[kNumChannelsInPixel * i]);
        color.y = ApplyGamma(data[kNumChannelsInPixel * i + 1]);
        color.z = ApplyGamma(data[kNumChannelsInPixel * i + 2]);

        m_pixels.emplace_back(std::move(color));
    }

    stbi_image_free(data);
}

void
Texture2D::Save(std::string path, ImageTypes imageType) const
{
    // TODO: png以外も実装
    switch (imageType)
    {
    case ImageTypes::Png:
    {
        std::vector<uint8_t> outPixels;
        outPixels.reserve(m_width * m_height);

        for (auto& pixel : m_pixels)
        {
            int degammaR = ApplyDegamma(pixel.x);
            int degammaG = ApplyDegamma(pixel.y);
            int degammaB = ApplyDegamma(pixel.z);

            outPixels.emplace_back(static_cast<uint8_t>(degammaR));
            outPixels.emplace_back(static_cast<uint8_t>(degammaG));
            outPixels.emplace_back(static_cast<uint8_t>(degammaB));
        }

        stbi_write_png(path.c_str(),
                       m_width,
                       m_height,
                       kNumChannelsInPixel,
                       &outPixels[0],
                       m_width * kNumChannelsInPixel);
        break;
    }

    case ImageTypes::Jpg:
    {
        UNIMPLEMENTED();
        break;
    }

    default:
    {
        ASSERT(false && "Invalid image type.");
        break;
    }
    }
}

void
Texture2D::Clear(const Color3f& color)
{
    m_pixels.resize(m_width * m_height, color);
}

Color3f
Texture2D::GetPixel(int i, int j) const
{
    /*if (i < 0)
    {
        i = static<int>(Math::Mod(i, m_width));
    }

    if (i >= m_width)
    {
        i = m_width - 1;
    }

    if (j < 0)
    {
        j = 0;
    }

    if (j >= m_height)
    {
        j = m_height - 1;
    }*/

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
    return GetPixel(u * (m_width - 1), v * (m_height - 1), interpoplationType);
}

void
Texture2D::SetPixel(int i, int j, const Color3f& color)
{
    ASSERT(0 <= i && i < m_width);
    ASSERT(0 <= j && j < m_height);
    m_pixels[m_width * j + i] = color;
}

int
Texture2D::GetWidth() const
{
    return m_width;
}

int
Texture2D::GetHeight() const
{
    return m_height;
}

} // namespace Core
} // namespace Petrichor
