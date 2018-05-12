#pragma once

#include <vector>
#include <Core/Geometry/Vertex.h>
#include <Core/Geometry/Triangle.h>

namespace Petrichor
{
namespace Core
{

class Scene;
class MaterialBase;

class Mesh
{
public:
    void Load(
        const std::string& path,
        MaterialBase** ppMaterial,
        size_t numMaterials,
        ShadingTypes shadingType = ShadingTypes::Flat
    );

    const std::vector<Triangle>&
    GetTriangles() const
    {
        return m_triangles;
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;

};


}
}
