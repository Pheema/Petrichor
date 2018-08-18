#pragma once

#include "ISampler1D.h"
#include "Random/XorShift.h"

namespace Petrichor
{
namespace Core
{

class RandomSampler1D : public ISampler1D
{
public:
    explicit RandomSampler1D(unsigned seed)
      : m_xorShift(seed){};

    float
    Next() final
    {
        return m_xorShift.next();
    }

private:
    Math::XorShift128 m_xorShift;
};

} // namespace Core
} // namespace Petrichor
