#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace Petrichor
{
namespace Math
{

inline bool
ApproxEq(float c0, float c1, float eps = std::numeric_limits<float>().epsilon())
{
    return std::abs(c0 - c1) < eps;
}

inline float
Sign(float x)
{
    return x >= 0.0f ? 1.0f : -1.0f;
}

template<typename T>
T
Lerp(T v0, T v1, float t)
{
    return (1.0f - t) * v0 + t * v1;
}

template<typename T>
T
Mod(T x, T m)
{
    return x - m * std::floor(x / m);
}

} // namespace Math
} // namespace Petrichor
