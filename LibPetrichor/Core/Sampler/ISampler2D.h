#pragma once

#include <tuple>

namespace Petrichor
{
namespace Core
{

class ISampler2D
{
public:
    virtual std::tuple<float, float>
    SampleNext2D() = 0;
};

} // namespace Core
} // namespace Petrichor
