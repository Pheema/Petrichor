#include "Vertex.h"

namespace Petrichor
{
namespace Core
{

Petrichor::Core::Vertex::Vertex()
  : pos()
  , normal()
  , uv()
{
}

Vertex::Vertex(float x, float y, float z)
  : pos(Math::Vector3f(x, y, z))
  , normal()
  , uv()
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
