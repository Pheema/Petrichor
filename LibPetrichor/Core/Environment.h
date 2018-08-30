#pragma once

#include "Core/Sampler/ISampler2D.h"
#include "Texture2D.h"

namespace Petrichor
{
namespace Core
{
class Environment
{
public:
    Environment() = default;

    //! 画像を読み込む
    void
    Load(std::string path);

    //! World座標系において、dir方向のテクセルをサンプリングする
    Color3f
    GetColor(const Math::Vector3f& dir) const;

    void
    SetBaseColor(const Color3f& baseColor);

    //! Z軸周りの回転を設定する
    void
    SetZAxisRotation(float angle)
    {
        m_ZAxisRotation = angle;
    }

    //! #TODO: 実装雑すぎるので後ほど修正
    //! 輝度の累積分布テクスチャを作成する
    void
    PreCalcCumulativeDistTex();

    //! 輝度に応じた環境マップの重点サンプリングする
    Math::Vector3f
    ImportanceSampling(ISampler2D& sampler2D, float* pdfXY);

    float
    GetImportanceSamplingPDF(const Math::Vector3f& dir) const;

    Texture2D m_debugTex;

    bool
    UseEnvImportanceSampling() const
    {
        return m_texEnv && m_texEnv->IsValid() && useEnvImportanceSampling;
    }

private:
    Texture2D* m_texEnv = nullptr; // #TODO: constにして外部からセット
    std::vector<float> m_pdf1D;    //!
    std::vector<float> m_cdf1D;    //!< #TODO: x軸方向の輝度CDF
    Texture2D m_pdf2D;
    Texture2D m_cdf2D;

    Color3f m_baseColor;
    float m_ZAxisRotation = 0.0f; //!< y軸周りの回転角[rad]
    bool useEnvImportanceSampling = true;
};
} // namespace Core
} // namespace Petrichor
