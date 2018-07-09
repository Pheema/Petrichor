#include "SweepBVH.h"

#include "Core/Accel/BVHNode.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Scene.h"
#include <stack>

namespace Petrichor
{
namespace Core
{

void
SweepBVH::Build(const Scene& scene)
{
    m_bvhNodes.clear();

    {
        BVHNode rootNode;
        rootNode.ReserveChildArray(scene.GetGeometries().size());
        for (const auto* geometry : scene.GetGeometries())
        {
            rootNode.bound.Merge(geometry->CalcBound());
            rootNode.AppendChild(geometry);
        }
        m_bvhNodes.emplace_back(std::move(rootNode));
    }

    std::stack<uint32_t> bvhNodeIndexStack;
    bvhNodeIndexStack.emplace(0);

    uint32_t index = 0;
    uint32_t depth = 1;

    while (!bvhNodeIndexStack.empty())
    {
        const uint32_t nodeIndex = bvhNodeIndexStack.top();
        bvhNodeIndexStack.pop();
        depth--;

        const auto& currentNode = m_bvhNodes[nodeIndex];
        auto geometryPtrs = currentNode.GetChildArray();
        const float invArea = currentNode.bound.GetSurfaceArea();

        const size_t numGeometryInNode = currentNode.GetNumChildGeoms();

        {
            float minSAH = std::numeric_limits<float>::infinity();
            float isLeaf = false;
            BVHNode childNodes[2];

            // ---- 各軸で分割 ----
            for (uint32_t axis = 0; axis < 3; axis++)
            {
                // 各パーティション位置において
                for (uint32_t i = 0; i < numGeometryInNode; i++)
                {
                    auto begin = std::begin(geometryPtrs);
                    auto mid = std::begin(geometryPtrs) + i;
                    auto end = std::end(geometryPtrs);

                    std::nth_element(
                      begin,
                      mid,
                      end,
                      [axis](const GeometryBase* lhs, const GeometryBase* rhs) {
                          return lhs->CalcBound().Center()[axis] <
                                 rhs->CalcBound().Center()[axis];
                      });

                    BVHNode node0;
                    for (auto iter = begin; iter != mid; iter++)
                    {
                        node0.bound.Merge((*iter)->CalcBound());
                        node0.AppendChild(*iter);
                    }

                    BVHNode node1;
                    for (auto iter = mid; iter != end; iter++)
                    {
                        node1.bound.Merge((*iter)->CalcBound());
                        node1.AppendChild(*iter);
                    }

                    constexpr float timeTri = 1.0f;
                    constexpr float timeAABB = 1.0f;

                    const size_t n0 = node0.GetNumChildGeoms();
                    const size_t n1 = node1.GetNumChildGeoms();

                    const float a0 = n0 ? node0.bound.GetSurfaceArea() : 0.0f;
                    const float a1 = n1 ? node1.bound.GetSurfaceArea() : 0.0f;

                    const float sah =
                      2.0f * timeAABB + (a0 * n0 + a1 * n1) * timeTri * invArea;

                    const bool willDivide =
                      (i != 0 && i != numGeometryInNode - 1);
                    if (sah < minSAH)
                    {
                        minSAH = sah;
                        isLeaf = !willDivide;

                        if (willDivide)
                        {
                            childNodes[0] = node0;
                            childNodes[1] = node1;
                        }
                    }
                }
            }

            if (isLeaf)
            {
                m_bvhNodes[nodeIndex].SetLeaf(true);
            }
            else
            {
                m_bvhNodes.emplace_back(childNodes[0]);
                m_bvhNodes.emplace_back(childNodes[1]);

                const size_t childIndicies[2]{ ++index, ++index };
                m_bvhNodes[nodeIndex].SetChildNode(0, childIndicies[0]);
                m_bvhNodes[nodeIndex].SetChildNode(1, childIndicies[1]);

                bvhNodeIndexStack.emplace(childIndicies[0]);
                bvhNodeIndexStack.emplace(childIndicies[1]);
                depth += 2;

                m_bvhNodes[nodeIndex].ClearAndShrink();
            }
        }
    }

    std::cout << "[Done] BVH Construction" << std::endl;
}

std::optional<Petrichor::Core::HitInfo>
SweepBVH::Intersect(const Ray& ray,
                    float distMin /*= 0.0f*/,
                    float distMax /*= kInfinity*/) const
{
    std::stack<size_t> bvhNodeIndexQueue;
    bvhNodeIndexQueue.emplace(0);

    // ---- BVHのトラバーサル ----
    std::optional<HitInfo> hitInfoResult;

    // 2つの子ノード又は子オブジェクトに対して
    while (!bvhNodeIndexQueue.empty())
    {
        // 葉ノードに対して
        const size_t bvhNodeIndex = bvhNodeIndexQueue.top();
        bvhNodeIndexQueue.pop();
        const BVHNode& currentNode = m_bvhNodes[bvhNodeIndex];

        const auto hitInfoNode = m_bvhNodes[bvhNodeIndex].Intersect(ray);

        // そもそもBVHノードに当たる軌道ではない
        if (hitInfoNode == std::nullopt)
        {
            continue;
        }

        // 自ノードより手前で既に衝突している
        if (hitInfoResult && currentNode.Contanins(ray.o) == false)
        {
            if (hitInfoResult->distance < hitInfoNode->distance)
            {
                continue;
            }
        }

        if (currentNode.IsLeaf())
        {
            for (const auto* geometry : currentNode.GetChildArray())
            {
                const auto hitInfoGeometry = geometry->Intersect(ray);
                if (hitInfoGeometry)
                {
                    if (hitInfoGeometry->distance < distMin ||
                        hitInfoGeometry->distance > distMax)
                    {
                        continue;
                    }

                    if (hitInfoResult == std::nullopt ||
                        hitInfoGeometry->distance < hitInfoResult->distance)
                    {
                        hitInfoResult = hitInfoGeometry;
                    }
                }
            }
        }
        else
        {
            // 枝ノードの場合
            for (const auto childNodeIndex : currentNode.GetChildNodeIndicies())
            {
                bvhNodeIndexQueue.emplace(childNodeIndex);
            }
        }
    }
    return hitInfoResult;
}
} // Core
} // Petrichor
