﻿#pragma once

#include "Core/Assert.h"
#include <vector>

namespace Petrichor
{
namespace Math
{

class Halton
{
public:
    Halton();

    void
    Initialize(size_t length, unsigned base);

    float
    GetValue(unsigned index) const
    {
        ASSERT(IsInitialized() && "Halton class is not initialized.");
        ASSERT(index < m_values.size());
        return m_values[index];
    }

    size_t
    GetLength() const
    {
        return m_values.size();
    }

    bool
    IsInitialized() const
    {
        return m_isInitialized;
    }

private:
    std::vector<float> m_values; // ハルトン列
    bool m_isInitialized = false;

    unsigned m_base = 0;
};

} // namespace Math
} // namespace Petrichor
