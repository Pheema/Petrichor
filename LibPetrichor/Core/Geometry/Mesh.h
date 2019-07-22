#pragma once

#include "Triangle.h"
#include "Vertex.h"
#include <filesystem>
#include <vector>

namespace Petrichor
{
namespace Core
{

class Scene;
class MaterialBase;

class Mesh
{
public:
    void
    Load(const std::filesystem::path& path,
         const MaterialBase* material,
         ShadingTypes shadingType = ShadingTypes::Flat);

    const std::vector<Triangle>&
    GetTriangles() const
    {
        return m_triangles;
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;
};
} // namespace Core
} // namespace Petrichor
