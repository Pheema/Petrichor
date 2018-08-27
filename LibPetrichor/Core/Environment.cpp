#include "Environment.h"

#include "Core/Color3f.h"
#include "Core/Constants.h"
#include "Math/Vector3f.h"
#include "Math/OrthonormalBasis.h"
#include "Math/MathUtils.h"

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

    m_pdf2D = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());
    m_cdf2D = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());

    // #DEBUG
    m_debugTex = Texture2D(m_texEnv->GetWidth(), m_texEnv->GetHeight());

    // 列方向の和
    Color3f sumLuminance2D;
    for (int i = 0; i < m_cdf2D.GetWidth(); i++)
    {
        for (int j = 0; j < m_cdf2D.GetHeight(); j++)
        {
            if (j == 0)
            {
                continue;
            }

            const Color3f& prevPixel = m_cdf2D.GetPixel(i, j - 1);
            const Color3f luminance = GetLuminance(m_texEnv->GetPixel(i, j)) * Color3f::One();
            sumLuminance2D += luminance;

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


    //// 行方向の和
    //for (int j = 0; j < m_cdf2D.GetHeight(); j++)
    //{
    //    for (int i = 0; i < m_cdf2D.GetWidth(); i++)
    //    {
    //        if (i == 0)
    //        {
    //            continue;
    //        }

    //        const float prevluminace = GetLuminance(m_cdf2D.GetPixel(i - 1, j));

    //        const float luminance = GetLuminance(m_cdf2D.GetPixel(i, j));

    //        m_cdf2D.SetPixel(i, j, (prevluminace + luminance) * Color3f::One());
    //    }
    //}

    /*Math::Vector3f sumOfAllPixel =
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
    }*/

    m_pdf2D.Save("pdf2D.hdr");
    m_cdf2D.Save("cdf2D.hdr");
}

Petrichor::Math::Vector3f Environment::SampleDir(float* pdfXY)
{
    // !!!
    const float u0 = rand() / static_cast<float>(RAND_MAX);
    const float u1 = rand() / static_cast<float>(RAND_MAX);

    float x0 = 0;
    float x1 = m_cdf1D.size() - 1;
    for (;;)
    {
        const float x = 0.5f * (x0 + x1);
        const float cdf1D = Math::Lerp(m_cdf1D[static_cast<int>(x)], m_cdf1D[static_cast<int>(x + 1)], x - static_cast<int>(x));
        const float diff = cdf1D - u0;

        if (x1 - x0 <= 1.0f)
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

    float y0 = 0;
    float y1 = m_cdf2D.GetHeight() - 1;
    for (;;)
    {
        const float y = 0.5f * (y0 + y1);
        const float diff = m_cdf2D.GetPixel(x0, y, Texture2D::InterplationTypes::Bilinear).x - u1;

        if (y1 - y0 <= 1.0f)
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

    if (pdfXY)
    {
        const float pdfX0 = m_pdf1D[x0];
        const float pdfY0UnderX0 = m_pdf2D.GetPixel(x0, y0, Texture2D::InterplationTypes::Bilinear).x;
        *pdfXY = pdfX0 * pdfY0UnderX0;
    }

    auto prevColor = m_debugTex.GetPixel(static_cast<int>(x0), static_cast<int>(y0));
    m_debugTex.SetPixel(static_cast<int>(x0), static_cast<int>(y0), prevColor + Color3f::One());

    const float theta = (x0 / m_pdf2D.GetWidth()) * Math::kPi;
    const float phi = 0.5f * Math::kPi * (1.0f - y0 / m_pdf2D.GetHeight()) - m_ZAxisRotation;

    Math::OrthonormalBasis onb;
    onb.Build(Math::Vector3f::UnitZ(), Math::Vector3f::UnitX());
    return onb.GetDir(theta, phi);
}

} // namespace Core
} // namespace Petrichor
