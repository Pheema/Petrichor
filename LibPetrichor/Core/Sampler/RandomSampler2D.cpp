#include "RandomSampler2D.h"

namespace Petrichor
{
namespace Core
{

RandomSampler2D::RandomSampler2D(unsigned seed) :
    m_xorShift(seed)
{
    // Do nothing
}

std::tuple<float, float> RandomSampler2D::SampleNext2D()
{
    float x = m_xorShift.next();
    float y = m_xorShift.next();
    return std::tuple<float, float>(x, y);
}

}   // namespace Core
}   // namespace Petrichor
