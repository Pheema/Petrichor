#include "Vertex.h"

namespace Petrichor
{
namespace Core
{

Vertex::Vertex(float x, float y, float z)
  : pos{ x, y, z }
{
}

Vertex::Vertex(const Math::Vector3f& pos,
               const Math::Vector3f& normal,
               const Math::Vector3f& uv)
  : pos(pos)
  , normal(normal)
  , uv(uv)
{
}

} // namespace Core
} // namespace Petrichor
