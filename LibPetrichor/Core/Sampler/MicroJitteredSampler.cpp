#include "MicroJitteredSampler.h"

#include "Math/MathUtils.h"

namespace Petrichor
{
namespace Core
{

MicroJitteredSampler::MicroJitteredSampler(unsigned seed)
  : m_index(0)
  , m_lengthSeq(0)
  , m_mu(0.0f)
  , m_offsetX(0.0f)
  , m_offsetY(0.0f)
  , m_xorShift(seed)
{
}

void
MicroJitteredSampler::Initialize(int lengthSeq)
{
    m_lengthSeq = lengthSeq;
    m_mu        = kStarDiscrepancy / sqrt(static_cast<float>(lengthSeq));
    m_haltonBase2.Initialize(m_lengthSeq, 2);
    m_haltonBase3.Initialize(m_lengthSeq, 3);
}

std::tuple<float, float>
MicroJitteredSampler::SampleNext2D()
{
    float x = m_haltonBase2.GetValue(m_index);
    float y = m_haltonBase3.GetValue(m_index);

    x += m_offsetX;
    y += m_offsetY;
    x = Math::Mod(x, 1.0f);
    y = Math::Mod(y, 1.0f);

    m_index++;
    if (m_index >= m_lengthSeq)
    {
        m_offsetX = m_mu * (m_xorShift.next() - 0.5f);
        m_offsetY = m_mu * (m_xorShift.next() - 0.5f);
        m_index   = 0;
    }
    return std::tuple<float, float>(x, y);
}

} // namespace Core
} // namespace Petrichor
