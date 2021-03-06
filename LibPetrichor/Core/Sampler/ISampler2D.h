﻿#pragma once

#include <tuple>

namespace Petrichor
{
namespace Core
{

class ISampler2D
{
public:
    virtual std::tuple<float, float>
    Next() = 0;
};

} // namespace Core
} // namespace Petrichor
