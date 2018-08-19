#pragma once

#include "ISampler2D.h"
#include "Random/XorShift.h"

namespace Petrichor
{
namespace Core
{

class RandomSampler2D : public ISampler2D
{
public:
    explicit RandomSampler2D(unsigned seed0, unsigned seed1);

    std::tuple<float, float>
    Next() final;

private:
    Math::XorShift128 m_xorShift0;
    Math::XorShift128 m_xorShift1;
};

} // namespace Core
} // namespace Petrichor
