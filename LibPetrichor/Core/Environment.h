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

    // 画像を読み込む
    void
    Load(std::string path);

    Color3f
    GetColor(const Math::Vector3f& dir) const;

    void
    SetBaseColor(const Color3f& baseColor);

private:
    Texture2D* m_texEnv = nullptr;
    Color3f m_baseColor;
};
} // namespace Core
} // namespace Petrichor
