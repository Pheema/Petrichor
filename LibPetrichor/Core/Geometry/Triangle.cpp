#include "Triangle.h"

#include "Core/Geometry/Vertex.h"
#include "Core/HitInfo.h"
#include "Core/Ray.h"
#include "Core/Sampler/ISampler2D.h"
#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

Triangle::Triangle(ShadingTypes shadingType)
  : m_vertices()
  , m_shadingType(shadingType)
{
}

Triangle::Triangle(const Vertex* v0,
                   const Vertex* v1,
                   const Vertex* v2,
                   ShadingTypes shadingType)
  : m_shadingType(shadingType)
{
    SetVertices(v0, v1, v2);
}

void
Triangle::SetVertices(const Vertex* v0, const Vertex* v1, const Vertex* v2)
{
    m_vertices[0] = v0;
    m_vertices[1] = v1;
    m_vertices[2] = v2;
}

Bound
Triangle::CalcBound() const
{
    Bound bound;
    for (const auto& vertex : m_vertices)
    {
        bound.Expand(vertex->pos);
    }
    m_bound = bound;
    return m_bound;
}

bool
Triangle::Intersect(const Ray& ray, HitInfo* hitInfo) const
{
    Math::Vector3f e1 = m_vertices[1]->pos - m_vertices[0]->pos;
    Math::Vector3f e2 = m_vertices[2]->pos - m_vertices[0]->pos;

    const auto crossEdges = Cross(e1, e2);
    const float invDet    = 1.0f / Dot(-ray.dir, crossEdges);

    auto vec = ray.o - m_vertices[0]->pos;

    float weightE1 = Dot(vec, Cross(ray.dir, e2)) * invDet;
    float weightE2 = Dot(vec, Cross(e1, ray.dir)) * invDet;

    if (weightE1 < 0.0f || weightE1 >= 1.0f)
    {
        return false;
    }
    if (weightE2 < 0.0f || weightE2 >= 1.0f)
    {
        return false;
    }
    if (weightE1 + weightE2 >= 1.0f)
    {
        return false;
    }

    float dist = Dot(vec, crossEdges) * invDet;
    if (dist < 0.0f)
    {
        return false;
    }

    hitInfo->distance = dist;
    hitInfo->pos      = ray.o + ray.dir * hitInfo->distance;

    switch (m_shadingType)
    {
    case ShadingTypes::Flat:
    {
        hitInfo->normal = crossEdges.Normalized();
        break;
    }

    case ShadingTypes::Smooth:
    {
        hitInfo->normal =
          weightE1 * (m_vertices[1]->normal - m_vertices[0]->normal) +
          weightE2 * (m_vertices[2]->normal - m_vertices[0]->normal) +
          m_vertices[0]->normal;
        hitInfo->normal.Normalize();
        break;
    }

    default:
    {
        ASSERT(false);
        break;
    }
    }

    hitInfo->uv = weightE1 * (m_vertices[1]->uv - m_vertices[0]->uv) +
                  weightE2 * (m_vertices[2]->uv - m_vertices[0]->uv) +
                  m_vertices[0]->uv;

    hitInfo->hitObj = this;
    return true;
}

void
Triangle::SampleSurface(Math::Vector3f p,
                        ISampler2D& sampler2D,
                        PointData* pointData,
                        float* pdfArea) const
{
    const Math::Vector3f e0 = m_vertices[1]->pos - m_vertices[0]->pos;
    const Math::Vector3f e1 = m_vertices[2]->pos - m_vertices[0]->pos;

    auto rand2D = sampler2D.Next();

    const float u0 = std::get<0>(rand2D);
    const float u1 = std::get<1>(rand2D);

    float sqrtu0 = sqrt(u0);
    const auto v = m_vertices[0]->pos + (1.0f - sqrtu0) * e0 + sqrtu0 * u1 * e1;

    pointData->pos    = v;
    pointData->normal = Math::Cross(e0, e1).Normalized();

    *pdfArea = 1.0f / (0.5f * Math::Cross(e0, e1).Length());
}

} // namespace Core
} // namespace Petrichor
