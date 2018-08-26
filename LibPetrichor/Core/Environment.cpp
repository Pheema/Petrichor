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
    const float phi = atan2(dir.y, dir.x) - m_yAxisRot;

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

} // namespace Core
} // namespace Petrichor
