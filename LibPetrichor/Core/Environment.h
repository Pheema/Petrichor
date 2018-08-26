#pragma once

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

private:
    Texture2D* m_texEnv = nullptr;
    Color3f m_baseColor;
    float m_ZAxisRotation = 0.0f; //!< y軸周りの回転角[rad]
};
} // namespace Core
} // namespace Petrichor
