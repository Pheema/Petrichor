#pragma once

namespace Petrichor
{
namespace Core
{

class ISampler1D
{
public:
    virtual float
    Next() = 0;
};

} // namespace Core
} // namespace Petrichor
