#include "RandomSampler2D.h"

namespace Petrichor
{
namespace Core
{

RandomSampler2D::RandomSampler2D(unsigned seed0, unsigned seed1)
  : m_xorShift0(seed0)
  , m_xorShift1(seed1)
{
    // Do nothing
}

std::tuple<float, float>
RandomSampler2D::Next()
{
    float x = m_xorShift0.next();
    float y = m_xorShift1.next();
    return std::tuple<float, float>(x, y);
}

} // namespace Core
} // namespace Petrichor
