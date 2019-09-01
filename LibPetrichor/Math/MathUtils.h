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

template<size_t N, typename T>
T
Pow(T x)
{
    static_assert(N >= 0 && std::is_arithmetic<T>::value);

    if constexpr (N == 0)
    {
        return static_cast<T>(1);
    }
    else if constexpr (N % 2 == 0)
    {
        return Pow<N / 2, T>(x) * Pow<N / 2, T>(x);
    }
    else
    {
        return x * Pow<N - 1, T>(x);
    }
}

} // namespace Math
} // namespace Petrichor
