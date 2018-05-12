#pragma once

#include <climits>
#include <random>

namespace Petrichor
{
namespace Math
{

// From: https://en.wikipedia.org/wiki/Xorshift
class XorShift128
{
public:
    XorShift128(){};
    explicit XorShift128(unsigned seed)
      : w(seed){};

    static constexpr unsigned
    min()
    {
        return 0u;
    }
    static constexpr unsigned
    max()
    {
        return UINT_MAX;
    }
    unsigned
    operator()()
    {
        return random();
    }

    unsigned
    random()
    {
        unsigned t;
        t = x ^ (x << 11);
        x = y;
        y = z;
        z = w;

        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }

    float
    next()
    {
        return random() / static_cast<float>(max());
    }

private:
    unsigned x = 123456789u;
    unsigned y = 362436069u;
    unsigned z = 521288629u;
    unsigned w = 486213789u;
};
}
}
