#include "AliasMethod.h"

#include <cmath>

namespace Petrichor
{
namespace Math
{

int
AliasMethod::Sample(float r0, float r1) const
{
    ASSERT(0.0f <= r0 && r0 < 1.0f);
    ASSERT(0.0f <= r1 && r1 < 1.0f);

    const auto index = static_cast<int>(r0 * m_boxColumns.size());

    const int result = r1 <= m_boxColumns[index].threshold
                         ? m_boxColumns[index].lowerIndex
                         : m_boxColumns[index].upperIndex;

    ASSERT(0 <= result && result < static_cast<int>(m_boxColumns.size()) &&
           "Invalid index");
    return result;
}

} // namespace Math
} // namespace Petrichor
