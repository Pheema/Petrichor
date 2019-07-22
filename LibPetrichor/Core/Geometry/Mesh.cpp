#include "Mesh.h"

#include "Core/Geometry/Triangle.h"
#include "Core/Geometry/Vertex.h"
#include "Core/Material/MaterialBase.h"
#include "Core/Scene.h"
#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
// #include "fmt/format.h"

namespace Petrichor
{
namespace Core
{

void
Mesh::Load(const std::filesystem::path& path,
           const MaterialBase* material,
           ShadingTypes shadingType /*= ShadingTypes::Flat*/
)
{
    using namespace Math;

    Assimp::Importer importer;

    // #TODO: 下記フラグが実際に有効かどうかを調査。
    /*unsigned importFlag = aiProcess_JoinIdenticalVertices |
                          aiProcess_ImproveCacheLocality |
                          aiProcess_Triangulate | aiProcess_OptimizeMeshes;*/

    unsigned importFlag = 0;

    const auto* aiscene = importer.ReadFile(path.string(), importFlag);
    if (aiscene == nullptr)
    {
        printf("Unable to load mesh: %s\n", importer.GetErrorString());
    }

    if (aiscene->HasMeshes())
    {
        // TODO: 現状は1メッシュのみ
        for (size_t idxMesh = 0; idxMesh < aiscene->mNumMeshes; ++idxMesh)
        {
            const aiMesh* pMesh = aiscene->mMeshes[idxMesh];
            fmt::print("[{}]: {}\n", path.string(), pMesh->mMaterialIndex);
            // ---- 頂点の読み込み ----
            for (size_t idxVert = 0; idxVert < pMesh->mNumVertices; ++idxVert)
            {
                Vertex v;
                if (pMesh->HasPositions())
                {
                    v.pos = Vector3f(pMesh->mVertices[idxVert].x,
                                     pMesh->mVertices[idxVert].y,
                                     pMesh->mVertices[idxVert].z);
                }

                // ---- 法線の読み込み ----
                if (pMesh->HasNormals())
                {
                    v.normal = Vector3f(pMesh->mNormals[idxVert].x,
                                        pMesh->mNormals[idxVert].y,
                                        pMesh->mNormals[idxVert].z)
                                 .Normalized();
                }

                // ---- UVの読み込み ----
                if (pMesh->HasTextureCoords(0))
                {
                    v.uv = Vector3f(pMesh->mTextureCoords[0][idxVert].x,
                                    pMesh->mTextureCoords[0][idxVert].y,
                                    0.0f);
                }

                m_vertices.emplace_back(std::move(v));
            }
        }

        // ---- 三角形の登録 ----
        uint32_t offset = 0;
        for (size_t idxMesh = 0; idxMesh < aiscene->mNumMeshes; ++idxMesh)
        {
            const aiMesh* pMesh = aiscene->mMeshes[idxMesh];
            if (pMesh->HasFaces())
            {
                for (size_t idxFace = 0; idxFace < pMesh->mNumFaces; ++idxFace)
                {
                    Triangle triangle(shadingType);
                    ASSERT(pMesh->mFaces[idxFace].mNumIndices == 3);
                    const auto* v0 =
                      &m_vertices[offset + pMesh->mFaces[idxFace].mIndices[0]];
                    const auto* v1 =
                      &m_vertices[offset + pMesh->mFaces[idxFace].mIndices[1]];
                    const auto* v2 =
                      &m_vertices[offset + pMesh->mFaces[idxFace].mIndices[2]];

                    /*std::cout << "[" <<
                        pMesh->mFaces[idxFace].mIndices[0] << ", " <<
                        pMesh->mFaces[idxFace].mIndices[1] << ", " <<
                        pMesh->mFaces[idxFace].mIndices[2] << ", " <<
                       std::endl;*/

                    ASSERT(offset + pMesh->mFaces[idxFace].mIndices[0] <
                           m_vertices.size());
                    ASSERT(offset + pMesh->mFaces[idxFace].mIndices[1] <
                           m_vertices.size());
                    ASSERT(offset + pMesh->mFaces[idxFace].mIndices[2] <
                           m_vertices.size());

                    triangle.SetVertices(v0, v1, v2);

                    // #TODO: 複数マテリアル
                    const size_t idxMaterial = pMesh->mMaterialIndex;
                    triangle.SetMaterial(material);

                    m_triangles.emplace_back(std::move(triangle));
                }

                offset += pMesh->mNumVertices;
            }
        }
    }
}
} // namespace Core
} // namespace Petrichor
