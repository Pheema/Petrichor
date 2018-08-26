#include "Environment.h"

#include "Core/Color3f.h"
#include "Core/Constants.h"
#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

void
Environment::Load(std::string path)
{
    if (m_texEnv == nullptr)
    {
        // TODO: メモリリーク
        m_texEnv = new Texture2D();
    }
    m_texEnv->Load(path, Texture2D::TextureColorType::Color);

    PreCalcCumulativeDistTex();
}

Color3f
Environment::GetColor(const Math::Vector3f& dir) const
{
    if (m_texEnv == nullptr)
    {
        return m_baseColor;
    }

    IS_NORMALIZED(dir);
    const float theta = acos(dir.z);
    const float phi = atan2(dir.y, dir.x) - m_ZAxisRotation;

    float u = 1.0f - phi * 0.5f * Math::kInvPi;
    float v = theta * Math::kInvPi;

    u = Math::Mod(u, 1.0f);
    v = Math::Mod(v, 1.0f);

    return m_baseColor * m_texEnv->GetPixelByUV(u, v);
}

void
Environment::SetBaseColor(const Color3f& baseColor)
{
    m_baseColor = baseColor;
}

void
Environment::PreCalcCumulativeDistTex()
{
    if (m_texEnv == nullptr)
    {
        return;
    }

    m_cdf2D = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());

    // 列方向の和
    for (int i = 0; i < m_cdf2D.GetWidth(); i++)
    {
        for (int j = 0; j < m_cdf2D.GetHeight(); j++)
        {
            if (j == 0)
            {
                continue;
            }

            const Color3f& prevPixel = m_cdf2D.GetPixel(i, j - 1);
            const Color3f luminance = m_texEnv->GetPixel(i, j);

            m_cdf2D.SetPixel(i, j, prevPixel + luminance);
        }
    }

    // cdf1Dの計算
    {
        m_cdf1D.clear();
        m_cdf1D.shrink_to_fit();
        m_cdf1D.reserve(m_cdf2D.GetWidth());
        float sumLuminance = 0.0f;
        for (int i = 0; i < m_cdf2D.GetWidth(); i++)
        {
            const int j = m_cdf2D.GetHeight() - 1;
            const float luminance = GetLuminance(m_cdf2D.GetPixel(i, j));
            sumLuminance += luminance;
            m_cdf1D.emplace_back(sumLuminance);
        }

        for (auto& elem : m_cdf1D)
        {
            elem /= sumLuminance;
        }
    }

    // 行方向の和
    for (int j = 0; j < m_cdf2D.GetHeight(); j++)
    {
        for (int i = 0; i < m_cdf2D.GetWidth(); i++)
        {
            if (i == 0)
            {
                continue;
            }

            const float prevluminace = GetLuminance(m_cdf2D.GetPixel(i - 1, j));

            const float luminance = GetLuminance(m_cdf2D.GetPixel(i, j));

            m_cdf2D.SetPixel(i, j, (prevluminace + luminance) * Color3f::One());
        }
    }

    Math::Vector3f sumOfAllPixel =
      m_cdf2D.GetPixel(m_cdf2D.GetWidth() - 1, m_cdf2D.GetHeight() - 1);

    m_maxLuminance = 0.0f;
    for (int j = 0; j < m_cdf2D.GetHeight(); j++)
    {
        for (int i = 0; i < m_cdf2D.GetWidth(); i++)
        {
            const float l = GetLuminance(m_texEnv->GetPixel(i, j));
            if (l > m_maxLuminance)
            {
                m_maxLuminance = l;
            }

            const Color3f pixel = m_cdf2D.GetPixel(i, j);
            m_cdf2D.SetPixel(i, j, pixel / sumOfAllPixel);
        }
    }
}

Color3f
Environment::ImportanceSampling(ISampler2D& sampler2D)
{
    const auto [u0, u1] = sampler2D.Next();

    int x0 = 0;
    int x1 = m_cdf2D.GetWidth() - 1;
    for (;;)
    {
        int x = (x0 + x1) / 2;
        const float diff = m_cdf2D.GetPixel(x, 0).x - u0;

        if (x1 - x0 <= 1)
        {
            x0 = x1 = x;
            break;
        }

        if (diff >= 0)
        {
            x1 = x;
        }
        else
        {
            x0 = x;
        }
    }

    int y0 = 0;
    int y1 = m_cdf2D.GetHeight() - 1;
    for (;;)
    {
        const int y = (y0 + y1) / 2;
        const float diff = m_cdf2D.GetPixel(x0, y).x - u1;

        if (y1 - y0 <= 1)
        {
            y0 = y1 = y;
            break;
        }

        if (diff >= 0)
        {
            y1 = y;
        }
        else
        {
            y0 = y;
        }
    }

    return m_cdf2D.GetPixel(x0, y0);
}

} // namespace Core
} // namespace Petrichor
