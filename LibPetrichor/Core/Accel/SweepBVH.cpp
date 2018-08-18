#include "SweepBVH.h"

#include "Core/Accel/BVHNode.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Scene.h"
#include <execution>
#include <stack>

namespace Petrichor
{
namespace Core
{

void
SweepBVH::Build(const Scene& scene)
{
    m_scene = &scene;

    m_bvhNodes.clear();
    m_entityIDs.clear();

    {
        Bounds rootBounds;
        uint32_t boundsID = 0;
        for (const auto* geometry : scene.GetGeometries())
        {
            Bounds bounds = geometry->CalcBound();
            m_bounds.emplace_back(bounds);
            m_entityIDs.emplace_back(boundsID++);
            rootBounds.Merge(bounds);
        }

        BVHNode::LeafNodeData leafNodeData{
            rootBounds, 0, static_cast<uint32_t>(scene.GetGeometries().size())
        };

        m_bvhNodes.emplace_back(leafNodeData);
    }

    std::stack<BVHNode*> bvhNodeStack;
    bvhNodeStack.emplace(&m_bvhNodes[0]);

    uint32_t depth = 1;

    while (!bvhNodeStack.empty())
    {
        auto* currentNode = bvhNodeStack.top();
        bvhNodeStack.pop();
        depth--;

        const auto idxBoundsBegin = currentNode->GetIndexOffset();
        const auto idxBoundsEnd =
          idxBoundsBegin + currentNode->GetNumPrimitives();

        {
            float minSAH = std::numeric_limits<float>::infinity();
            bool isLeaf = false;
            BVHNode::LeafNodeData leafNodesData[2];

            // ---- 各軸で分割 ----
            for (uint32_t axis = 0; axis < 3; axis++)
            {
                // 各パーティション位置において
                for (uint32_t idxBoundsMid = idxBoundsBegin;
                     idxBoundsMid < idxBoundsEnd;
                     idxBoundsMid++)
                {
                    std::nth_element(
                      std::execution::par,
                      &m_bounds[idxBoundsBegin],
                      &m_bounds[idxBoundsMid],
                      &m_bounds[idxBoundsBegin] +
                        currentNode->GetNumPrimitives(),
                      [axis](const Bounds& lhs, const Bounds& rhs) {
                          return lhs.GetCenter()[axis] < rhs.GetCenter()[axis];
                      });

                    std::sort(std::execution::par,
                              &m_entityIDs[idxBoundsBegin],
                              &m_entityIDs[idxBoundsBegin] +
                                currentNode->GetNumPrimitives(),
                              [axis, this](uint32_t lhs, uint32_t rhs) {
                                  return m_bounds[lhs].GetCenter()[axis] <
                                         m_bounds[rhs].GetCenter()[axis];
                              });

                    Bounds bounds0;
                    for (auto idx = idxBoundsBegin; idx < idxBoundsMid; idx++)
                    {
                        bounds0.Merge(m_bounds[idx]);
                    }

                    Bounds bounds1;
                    for (auto idx = idxBoundsMid; idx < idxBoundsEnd; idx++)
                    {
                        bounds1.Merge(m_bounds[idx]);
                    }

                    const auto n0 = idxBoundsMid - idxBoundsBegin;
                    const auto n1 = idxBoundsEnd - idxBoundsMid;

                    const float a0 = bounds0.GetSurfaceArea();
                    const float a1 = bounds1.GetSurfaceArea();

                    const float sah = a0 * n0 + a1 * n1;

                    const bool willDivide =
                      (idxBoundsMid != 0 && idxBoundsMid != idxBoundsEnd - 1);
                    if (sah < minSAH)
                    {
                        minSAH = sah;
                        isLeaf = !willDivide;

                        if (willDivide)
                        {
                            leafNodesData[0] = BVHNode::LeafNodeData{
                                bounds0, idxBoundsBegin, n0
                            };

                            leafNodesData[1] = BVHNode::LeafNodeData{
                                bounds1, idxBoundsMid, n1
                            };
                        }
                    }
                }
            }

            if (isLeaf == false)
            {
                std::array<BVHNode*, 2> childNodes{
                    &m_bvhNodes.emplace_back(leafNodesData[0]),
                    &m_bvhNodes.emplace_back(leafNodesData[1])
                };

                BVHNode::InternalNodeData internalNodeData{
                    currentNode->GetBounds(), childNodes
                };

                *currentNode = BVHNode(internalNodeData);

                bvhNodeStack.emplace(childNodes[0]);
                bvhNodeStack.emplace(childNodes[1]);
                depth += 2;
            }
        }
    }

    std::cout << "[Done] BVH Construction" << std::endl;
}

std::optional<HitInfo>
SweepBVH::Intersect(const Ray& ray,
                    float distMin /*= 0.0f*/,
                    float distMax /*= kInfinity*/) const
{
    std::stack<const BVHNode*> bvhNodeStack;
    bvhNodeStack.emplace(&m_bvhNodes[0]);

    // ---- BVHのトラバーサル ----
    std::optional<HitInfo> hitInfoResult;

    // 2つの子ノード又は子オブジェクトに対して
    while (!bvhNodeStack.empty())
    {
        // 葉ノードに対して
        const auto* currentNode = bvhNodeStack.top();
        bvhNodeStack.pop();

        const auto hitInfoNode = currentNode->Intersect(ray);

        // そもそもBVHノードに当たる軌道ではない
        if (hitInfoNode == std::nullopt)
        {
            continue;
        }

        // 自ノードより手前で既に衝突している
        if (hitInfoResult && currentNode->Contains(ray.o) == false)
        {
            if (hitInfoResult->distance < hitInfoNode->distance)
            {
                continue;
            }
        }

        if (currentNode->IsLeaf())
        {
            // for (const auto* geometry : currentNode.GetChildArray())
            for (uint32_t idx = currentNode->GetIndexOffset();
                 idx < currentNode->GetIndexOffset() +
                         currentNode->GetNumPrimitives();
                 idx++)
            {
                const GeometryBase* const geometry =
                  m_scene->GetGeometries()[m_entityIDs[idx]];

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
            for (const auto childNode : currentNode->GetChildNodes())
            {
                bvhNodeStack.emplace(childNode);
            }
        }
    }
    return hitInfoResult;
}
} // namespace Core
} // namespace Petrichor
