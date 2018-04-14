#pragma once

#include "ISampler2D.h"
#include <Math/XorShift.h>

namespace Petrichor
{
namespace Core
{

class RandomSampler2D : public ISampler2D
{
public:
    RandomSampler2D(unsigned seed);

    virtual std::tuple<float, float> SampleNext2D() final;

private:
    Math::XorShift128 m_xorShift;

};

}   // namespace Core
}   // namespace Petrichor
