#include "Environment.h"

#include "Core/Color3f.h"
#include "Core/Constants.h"
#include "Math/MathUtils.h"
#include "Math/OrthonormalBasis.h"
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
    const float phi = atan2(dir.y, dir.x);

    float u = 1.0f - (phi - m_ZAxisRotation) * 0.5f * Math::kInvPi;
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

    m_pdf2D = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());
    m_cdf2D = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());

    // #DEBUG
    m_debugTex = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());

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
            const Color3f luminance =
              GetLuminance(m_texEnv->GetPixel(i, j)) * Color3f::One();

            m_pdf2D.SetPixel(i, j, luminance);
            m_cdf2D.SetPixel(i, j, prevPixel + luminance);
        }
    }

    // cdf1Dの計算
    {
        m_pdf1D.clear();
        m_pdf1D.shrink_to_fit();
        m_pdf1D.reserve(m_cdf2D.GetWidth());
        m_cdf1D.clear();
        m_cdf1D.shrink_to_fit();
        m_cdf1D.reserve(m_cdf2D.GetWidth());

        float sumLuminance1D = 0.0f;
        const int jMax = m_cdf2D.GetHeight() - 1;
        for (int i = 0; i < m_cdf2D.GetWidth(); i++)
        {
            const float luminance = m_cdf2D.GetPixel(i, jMax).x;
            m_pdf1D.emplace_back(luminance);
            sumLuminance1D += luminance;
            m_cdf1D.emplace_back(sumLuminance1D);
        }

        for (int i = 0; i < m_cdf2D.GetWidth(); i++)
        {
            m_pdf1D[i] /= sumLuminance1D;
            m_cdf1D[i] /= sumLuminance1D;
        }
    }

    {
        const int jMax = m_cdf2D.GetHeight() - 1;
        for (int i = 0; i < m_cdf2D.GetWidth(); i++)
        {
            const float luminance = m_cdf2D.GetPixel(i, jMax).x;
            Color3f maxL = m_cdf2D.GetPixel(i, jMax);

            for (int j = 0; j < m_cdf2D.GetHeight(); j++)
            {
                Color3f l = m_pdf2D.GetPixel(i, j);
                m_pdf2D.SetPixel(i, j, l / maxL);

                Color3f integratedL = m_cdf2D.GetPixel(i, j);
                m_cdf2D.SetPixel(i, j, integratedL / maxL);
            }
        }
    }
}

Math::Vector3f
Environment::ImportanceSampling(ISampler2D& sampler2D, float* pdfXY)
{
    const float texelWidth = 1.0f / m_pdf2D.GetWidth();
    const float texelHeight = 1.0f / m_pdf2D.GetHeight();

    const auto [rand0, rand1] = sampler2D.Next();

    float u0 = 0.0f;
    float u1 = 1.0f;
    for (;;)
    {
        const float u = 0.5f * (u0 + u1);
        const float x = u * m_cdf2D.GetWidth();
        const auto floorX = static_cast<int>(x);
        const auto ceilX = std::min(floorX + 1, m_cdf2D.GetWidth() - 1);
        const float cdf1D =
          Math::Lerp(m_cdf1D[floorX], m_cdf1D[ceilX], x - floorX);
        const float diff = cdf1D - rand0;

        if (u1 - u0 <= texelWidth)
        {
            u0 = u1 = u;
            break;
        }

        if (diff >= 0)
        {
            u1 = u;
        }
        else
        {
            u0 = u;
        }
    }

    float v0 = 0.0f;
    float v1 = 1.0f;
    for (;;)
    {
        const float v = 0.5f * (v0 + v1);
        const float diff =
          m_cdf2D.GetPixelByUV(u0, v, Texture2D::InterplationTypes::Bilinear)
            .x -
          rand1;

        if (v1 - v0 <= texelHeight)
        {
            v0 = v1 = v;
            break;
        }

        if (diff > 0)
        {
            v1 = v;
        }
        else if (diff < 0)
        {
            v0 = v;
        }
        else
        {
            v0 = v1 = v;
            break;
        }
    }

    // u0 += 0.5f * texelWidth;
    // v0 += 0.5f * texelHeight;

    if (pdfXY)
    {
        const float x = u0 * m_pdf2D.GetWidth();
        const auto x0 = static_cast<int>(x);
        const int x1 = std::min(x0 + 1, static_cast<int>(m_pdf1D.size()) - 1);
        const float pdfX0 = Math::Lerp(m_pdf1D[x0], m_pdf1D[x1], x - x0);
        const float pdfY0UnderX0 =
          m_pdf2D.GetPixelByUV(u0, v0, Texture2D::InterplationTypes::Bilinear)
            .x;
        *pdfXY = pdfX0 * pdfY0UnderX0;
    }

    auto prevColor =
      m_debugTex.GetPixelByUV(u0, v0, Texture2D::InterplationTypes::Point);
    m_debugTex.SetPixel(static_cast<int>(u0 * m_debugTex.GetWidth()),
                        static_cast<int>(v0 * m_debugTex.GetHeight()),
                        prevColor + Color3f::One());

    const float theta = v0 * Math::kPi;
    const float phi = 2.0f * Math::kPi * (1.0f - u0) + m_ZAxisRotation;

    Math::OrthonormalBasis onb;
    onb.Build(Math::Vector3f::UnitZ(), Math::Vector3f::UnitX());
    return onb.GetDir(theta, phi);
}

float
Environment::GetImportanceSamplingPDF(const Math::Vector3f& dir) const
{
    if (m_texEnv == nullptr)
    {
        return 0.0f;
    }

    IS_NORMALIZED(dir);
    const float theta = acos(dir.z);
    const float phi = atan2(dir.y, dir.x);

    float u0 = 1.0f - (phi - m_ZAxisRotation) * 0.5f * Math::kInvPi;
    float v0 = theta * Math::kInvPi;

    const float x = u0 * m_pdf2D.GetWidth();
    const auto x0 = static_cast<int>(x);
    const int x1 = std::min(x0 + 1, static_cast<int>(m_pdf1D.size()) - 1);
    const float pdfX0 = Math::Lerp(m_pdf1D[x0], m_pdf1D[x1], x - x0);
    const float pdfY0UnderX0 =
      m_pdf2D.GetPixelByUV(u0, v0, Texture2D::InterplationTypes::Bilinear).x;
    return pdfX0 * pdfY0UnderX0;
}

} // namespace Core
} // namespace Petrichor
