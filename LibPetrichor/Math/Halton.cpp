#include "Halton.h"

#include <random>

namespace Petrichor
{
namespace Math
{

Halton::Halton()
  : m_isInitialized(false)
{
    // Do nothing
}

void
Halton::Initialize(size_t length, unsigned base)
{
    m_base = base;
    m_values.resize(length);

    // TODO: naiveな実装なので工夫する
    for (unsigned i = 0; i < length; i++)
    {
        unsigned index = i;
        float f = 1.0f, r = 0.0f;
        while (index > 0)
        {
            f /= m_base;
            r += f * (index % m_base);
            index /= m_base;
        }
        m_values[i] = r;
    }

    m_isInitialized = true;
}

} // namepsace Math
} // Petrichor
