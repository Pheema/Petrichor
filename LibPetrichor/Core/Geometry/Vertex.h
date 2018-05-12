#pragma once

#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

struct Vertex
{
    Vertex();

    Vertex(float x, float y, float z);

    Vertex(const Math::Vector3f& pos,
           const Math::Vector3f& normal,
           const Math::Vector3f& uv);

    Math::Vector3f pos;
    Math::Vector3f normal = Math::Vector3f::UnitZ();
    Math::Vector3f uv     = Math::Vector3f::Zero();
};

} // namespace Core
} // namespace Petrichor
