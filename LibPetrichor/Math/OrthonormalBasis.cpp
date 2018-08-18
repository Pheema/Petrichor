#include "OrthonormalBasis.h"

#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Math
{

void
OrthonormalBasis::Build(const Vector3f& w)
{
    m_w = w.Normalized();
    float sign = std::copysign(1.0f, m_w.z);
    const float a = -1.0f / (sign + m_w.z);
    const float b = m_w.x * m_w.y * a;

    m_u = Vector3f(1.0f + sign * m_w.x * m_w.x * a, sign * b, -sign * m_w.x);

    m_v = Vector3f(b, sign + m_w.y * m_w.y * a, -m_w.y);

    // TODO: テスト側に移したい
    IS_APPROXEQ(Dot(m_u, m_v), 0.0f);
    IS_APPROXEQ(Dot(m_v, m_w), 0.0f);
    IS_APPROXEQ(Dot(m_w, m_u), 0.0f);
}

void
OrthonormalBasis::Build(const Math::Vector3f& w, const Math::Vector3f& u)
{
    m_w = w.Normalized();
    m_u = u.Normalized();
    m_v = Math::Cross(m_w, m_u);
}

Math::Vector3f
OrthonormalBasis::GetDir(float theta, float phi)
{
    const float coffX = sin(theta) * cos(phi);
    const float coffY = sin(theta) * sin(phi);
    const float coffZ = cos(theta);

    return coffX * GetU() + coffY * GetV() + coffZ * GetW();
}

Vector3f
OrthonormalBasis::WorldToLocal(const Vector3f& vecWorld) const
{
    const float localX = Dot(vecWorld, GetU());
    const float localY = Dot(vecWorld, GetV());
    const float localZ = Dot(vecWorld, GetW());
    return Vector3f(localX, localY, localZ);
}

Vector3f
OrthonormalBasis::LocalToWorld(const Vector3f& vecLocal) const
{
    const auto vecWorld =
      m_u * vecLocal.x + m_v * vecLocal.y + m_w * vecLocal.z;

    return vecWorld;
}

} // namespace Math
} // namespace Petrichor
