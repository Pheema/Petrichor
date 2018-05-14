#include "OrthonormalBasis.h"

#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Math
{

OrthonormalBasis::OrthonormalBasis()
  : m_baseX()
  , m_baseY()
  , m_baseZ()
{
}

void
OrthonormalBasis::Build(const Vector3f& baseZ)
{
    m_baseZ       = baseZ.Normalized();
    float sign    = std::copysign(1.0f, m_baseZ.z);
    const float a = -1.0f / (sign + m_baseZ.z);
    const float b = m_baseZ.x * m_baseZ.y * a;

    m_baseX = Vector3f(
      1.0f + sign * m_baseZ.x * m_baseZ.x * a, sign * b, -sign * m_baseZ.x);

    m_baseY = Vector3f(b, sign + m_baseZ.y * m_baseZ.y * a, -m_baseZ.y);

    // TODO: テスト側に移したい
    IS_APPROXEQ(Dot(m_baseX, m_baseY), 0.0f);
    IS_APPROXEQ(Dot(m_baseY, m_baseZ), 0.0f);
    IS_APPROXEQ(Dot(m_baseZ, m_baseX), 0.0f);
}

void
OrthonormalBasis::Build(const Math::Vector3f& baseZ,
                        const Math::Vector3f& baseX)
{
    m_baseZ = baseZ.Normalized();
    m_baseX = baseX.Normalized();

    m_baseY = Math::Cross(m_baseZ, m_baseX);
}

Math::Vector3f
OrthonormalBasis::GetDir(float theta, float phi)
{
    const float coffX = sin(theta) * cos(phi);
    const float coffY = sin(theta) * sin(phi);
    const float coffZ = cos(theta);

    return coffX * GetBaseX() + coffY * GetBaseY() + coffZ * GetBaseZ();
}

Vector3f
OrthonormalBasis::WorldToLocal(const Vector3f& vecWorld) const
{
    const float localX = Dot(vecWorld, GetBaseX());
    const float localY = Dot(vecWorld, GetBaseY());
    const float localZ = Dot(vecWorld, GetBaseZ());
    return Vector3f(localX, localY, localZ);
}

Vector3f
OrthonormalBasis::LocalToWorld(const Vector3f& vecLocal) const
{
    const auto vecWorld =
      m_baseX * vecLocal.x + m_baseY * vecLocal.y + m_baseZ * vecLocal.z;

    return vecWorld;
}

} // namepsace Math
} // Petrichor
