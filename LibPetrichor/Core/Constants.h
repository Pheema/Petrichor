#pragma once

#include <limits>

namespace Petrichor
{

constexpr float kEps = 1.0e-3f;
constexpr float kEpsVec = 1.0e-3f;
constexpr float kInfinity = std::numeric_limits<float>::max();

namespace Math
{

constexpr float kPi = 3.14159265358979f;
constexpr float kInvPi = 1.0f / kPi;

}

}   // namespace Petrichor

