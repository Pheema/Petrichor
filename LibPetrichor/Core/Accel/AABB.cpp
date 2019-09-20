#include "AABB.h"

#include "Core/Constants.h"
#include <algorithm>

namespace Petrichor
{
namespace Core
{

void
AABB::Merge(const AABB& other)
{
    lower.x = std::min(lower.x, other.lower.x);
    lower.y = std::min(lower.y, other.lower.y);
    lower.z = std::min(lower.z, other.lower.z);

    upper.x = std::max(upper.x, other.upper.x);
    upper.y = std::max(upper.y, other.upper.y);
    upper.z = std::max(upper.z, other.upper.z);
}

void
AABB::Merge(const Math::Vector3f& point)
{
    lower.x = std::min(lower.x, point.x);
    lower.y = std::min(lower.y, point.y);
    lower.z = std::min(lower.z, point.z);

    upper.x = std::max(upper.x, point.x);
    upper.y = std::max(upper.y, point.y);
    upper.z = std::max(upper.z, point.z);
}

int
AABB::GetWidestAxis() const
{
    float widestWidth = 0.0f;
    int widestAxis = 0;

    for (int axis = 0; axis < 3; ++axis)
    {
        const float width = upper[axis] - lower[axis];
        if (width > widestWidth)
        {
            widestWidth = width;
            widestAxis = axis;
        }
    }

    return widestAxis;
}

} // namespace Core
} // namespace Petrichor
