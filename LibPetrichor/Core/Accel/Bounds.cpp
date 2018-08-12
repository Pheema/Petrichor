#include "Bounds.h"

#include "Core/Constants.h"
#include <algorithm>

namespace Petrichor
{
namespace Core
{

Bounds::Bounds()
  : vMin(Math::Vector3f::One() * kInfinity)
  , vMax(Math::Vector3f::One() * -kInfinity)
{
}

Bounds::Bounds(const Math::Vector3f& vMin, const Math::Vector3f& vMax)
  : vMin(vMin)
  , vMax(vMax)
{
}

float
Bounds::GetSurfaceArea() const
{
    auto diff = vMax - vMin;
    return 2.0f * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
}

void
Bounds::Merge(const Bounds& other)
{
    vMin.x = std::min(vMin.x, other.vMin.x);
    vMin.y = std::min(vMin.y, other.vMin.y);
    vMin.z = std::min(vMin.z, other.vMin.z);

    vMax.x = std::max(vMax.x, other.vMax.x);
    vMax.y = std::max(vMax.y, other.vMax.y);
    vMax.z = std::max(vMax.z, other.vMax.z);
}

void
Bounds::Merge(const Math::Vector3f& point)
{
    vMin.x = std::min(vMin.x, point.x);
    vMin.y = std::min(vMin.y, point.y);
    vMin.z = std::min(vMin.z, point.z);

    vMax.x = std::max(vMax.x, point.x);
    vMax.y = std::max(vMax.y, point.y);
    vMax.z = std::max(vMax.z, point.z);
}

uint8_t
Bounds::GetWidestAxis() const
{
    float widest = 0.0f;
    uint8_t axisResult = 0;

    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        const float width = vMax[axis] - vMin[axis];
        if (width > widest)
        {
            widest = width;
            axisResult = axis;
        }
    }

    return axisResult;
}

} // namespace Core
} // namespace Petrichor
