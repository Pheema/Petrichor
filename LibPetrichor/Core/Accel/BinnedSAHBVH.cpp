#include "BinnedSAHBVH.h"

#include "Core/Accel/BVHNode.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/Scene.h"
#include <numeric>
#include <stack>

namespace Petrichor
{
namespace Core
{

void
BinnedSAHBVH::Build(const Scene& scene)
{
    const auto numPrimitives = scene.GetGeometries().size();

    // 事前に全プリミティブのバウンディングボックスと重心を計算しておく
    {
        m_primitiveData.clear();
        m_primitiveData.reserve(numPrimitives);
        for (auto* primitive : scene.GetGeometries())
        {
            const Bounds bounds = primitive->GetBounds();
            const Math::Vector3f centroid = primitive->GetCentroid();
            PrimitiveData primitiveData{ -1, bounds, centroid };
            m_primitiveData.emplace_back(primitiveData);
        }
        m_primitiveData.shrink_to_fit();
    }

    std::stack<BVHNode*> nodeStack;
    // ルートのノードを計算
    {
        m_bvhNodes.clear();

        // メモリ再配置が起こると落ちるので、ノード数より大きい数を保証する
        // TODO: 最適な上限を求める
        m_bvhNodes.reserve(2ll * numPrimitives);

        Bounds rootBounds;
        for (auto* primitive : scene.GetGeometries())
        {
            rootBounds.Merge(primitive->GetBounds());
        }

        BVHNode& rootNode = m_bvhNodes.emplace_back(
          rootBounds, std::array{ -1, -1 }, 0, numPrimitives, false);

        nodeStack.emplace(&rootNode);
    }

    // InPlaceソート用プリミティブID配列を初期化
    {
        m_primitiveIDs.clear();
        m_primitiveIDs.resize(numPrimitives);
        std::iota(std::begin(m_primitiveIDs), std::end(m_primitiveIDs), 0);
        m_primitiveIDs.shrink_to_fit();
    }

    while (!nodeStack.empty())
    {
        BVHNode* currentNode = nodeStack.top();
        nodeStack.pop();

        const int indexBegin = currentNode->GetIndexBegin();
        const int indexEnd = currentNode->GetIndexEnd();
        const int numPrimitivesInCurrentNode = (indexEnd - indexBegin);

        // ノード内のプリミティブ数が十分に少ない場合は分割しない
        constexpr int kMinNumPrimitivesInNode = 4;
        if (numPrimitivesInCurrentNode <= kMinNumPrimitivesInNode)
        {
            currentNode->SetLeaf(true);
            continue;
        }

        // バウンディングボックスの一番長い辺に沿ってソート
        const auto iterBegin = std::begin(m_primitiveIDs) + indexBegin;
        const auto iterEnd = std::begin(m_primitiveIDs) + indexEnd;

        // ビンをアップデート
        Bounds binBounds;
        for (auto iter = iterBegin; iter != iterEnd; iter++)
        {
            const int primitiveID = *iter;
            binBounds.Merge(m_primitiveData[primitiveID].centroid);
        }

        const int widestAxis = binBounds.GetWidestAxis();

        std::sort(iterBegin, iterEnd, [this, widestAxis](int id0, int id1) {
            return m_primitiveData[id0].centroid[widestAxis] <
                   m_primitiveData[id1].centroid[widestAxis];
        });

        // どのビンに属しているかを番号付け
        const float widestEdgeLength =
          binBounds.vMax[widestAxis] - binBounds.vMin[widestAxis];

        for (auto iter = iterBegin; iter != iterEnd; iter++)
        {
            const int primitiveID = *iter;

            PrimitiveData* const primitiveData = &m_primitiveData[primitiveID];

            const float l =
              primitiveData->centroid[widestAxis] - binBounds.vMin[widestAxis];

            const auto binID =
              static_cast<int>(kNumBins * (1.0f - kEps) * l / widestEdgeLength);
            ASSERT(binID >= 0);
            primitiveData->binID = binID;
        }

        // 最適な分割位置を探索
        int bestBinPartitionIndex = 0;
        int bestNumPrimsInLeft = 0;
        float minCost = kInfinity;
        for (int binPartitionIndex = 0; binPartitionIndex <= kNumBins;
             binPartitionIndex++)
        {
            const auto [cost, numPrimsInLeft] = GetSAHCost(
              binPartitionIndex, *currentNode, m_primitiveData, m_primitiveIDs);

            if (cost < minCost)
            {
                minCost = cost;
                bestBinPartitionIndex = binPartitionIndex;
                bestNumPrimsInLeft = numPrimsInLeft;
            }
        }

        if (bestBinPartitionIndex == 0)
        {
            currentNode->SetLeaf(true);
            continue;
        }

        if (bestBinPartitionIndex == kNumBins)
        {
            currentNode->SetLeaf(true);
            continue;
        }

        // ---- 分割する場合 ----
        {
            Bounds leftBounds, rightBounds;

            for (auto iter = iterBegin; iter != iterEnd; iter++)
            {
                const int primitiveID = *iter;

                const PrimitiveData& primitiveData =
                  m_primitiveData[primitiveID];

                const bool isInLeftNode =
                  (primitiveData.binID < bestBinPartitionIndex);
                if (isInLeftNode)
                {
                    leftBounds.Merge(primitiveData.bounds);
                }
                else
                {
                    rightBounds.Merge(primitiveData.bounds);
                }
            }

            // Left child node
            {
                BVHNode& leftChildNode = m_bvhNodes.emplace_back(
                  leftBounds,
                  std::array{ -1, -1 },
                  currentNode->GetIndexBegin(),
                  currentNode->GetIndexBegin() + bestNumPrimsInLeft,
                  false);
                nodeStack.emplace(&leftChildNode);

                const auto index = static_cast<int>(m_bvhNodes.size() - 1);
                currentNode->SetChildNode(index, 0);
            }

            {
                BVHNode& rightChildNode = m_bvhNodes.emplace_back(
                  rightBounds,
                  std::array{ -1, -1 },
                  currentNode->GetIndexBegin() + bestNumPrimsInLeft,
                  currentNode->GetIndexEnd(),
                  false);
                nodeStack.emplace(&rightChildNode);

                const auto index = static_cast<int>(m_bvhNodes.size() - 1);
                currentNode->SetChildNode(index, 1);
            }
        }
    }
    fmt::print("[Done] BVH Construction\n");
}

std::pair<float, int>
BinnedSAHBVH::GetSAHCost(int binPartitionIndex,
                         const BVHNode& currentNode,
                         const std::vector<PrimitiveData>& primitiveDataArray,
                         const std::vector<int>& primitiveIDs)
{
    Bounds leftBounds, rightBounds;
    int numPrimsInLeft = 0, numPrimsInRight = 0;

    const auto iterBegin =
      std::begin(primitiveIDs) + currentNode.GetIndexBegin();
    const auto iterEnd = std::begin(primitiveIDs) + currentNode.GetIndexEnd();

    for (auto iter = iterBegin; iter != iterEnd; iter++)
    {
        const int primitiveID = *iter;

        const PrimitiveData* primitiveData = &primitiveDataArray[primitiveID];

        const int primitiveBinID = primitiveData->binID;
        if (primitiveBinID < binPartitionIndex)
        {
            // left
            leftBounds.Merge(primitiveData->bounds);
            numPrimsInLeft++;
        }
        else
        {
            // right
            rightBounds.Merge(primitiveData->bounds);
            numPrimsInRight++;
        }
    }

    const float surfaceAreaLeft =
      numPrimsInLeft ? leftBounds.GetSurfaceArea() : 0;

    const float surfaceAreaRight =
      numPrimsInRight ? rightBounds.GetSurfaceArea() : 0;

    const float cost =
      numPrimsInLeft * surfaceAreaLeft + numPrimsInRight * surfaceAreaRight;
    return { cost, numPrimsInLeft };
}

std::optional<HitInfo>
BinnedSAHBVH::Intersect(const Ray& ray,
                        const Scene& scene,
                        float distMin,
                        float distMax) const
{
    const PrecalcedData precalced = [&] {
        PrecalcedData precalced_;
        precalced_.invRayDir = Math::Vector3f::One() / ray.dir;
        precalced_.sign[0] = (precalced_.invRayDir[0] < 0.0f);
        precalced_.sign[1] = (precalced_.invRayDir[1] < 0.0f);
        precalced_.sign[2] = (precalced_.invRayDir[2] < 0.0f);
        return precalced_;
    }();

    thread_local std::vector<int> bvhNodeIndexStack;
    bvhNodeIndexStack.clear();
    bvhNodeIndexStack.emplace_back(0);

    // ---- BVHのトラバーサル ----
    std::optional<HitInfo> hitInfoResult;

    // 2つの子ノード又は子オブジェクトに対して
    while (!bvhNodeIndexStack.empty())
    {
        // 葉ノードに対して
        const auto index = static_cast<int>(bvhNodeIndexStack.size() - 1);
        const int currentNodeIndex = bvhNodeIndexStack[index];

        bvhNodeIndexStack.erase(std::end(bvhNodeIndexStack) - 1);
        const BVHNode& currentNode = m_bvhNodes[currentNodeIndex];

        const auto hitInfoNode = currentNode.Intersect(ray, precalced);

        // そもそもBVHノードに当たる軌道ではない
        if (!hitInfoNode)
        {
            continue;
        }

        // 自ノードより手前で既に衝突している
        if (hitInfoResult && currentNode.Contains(ray.o) == false)
        {
            if (hitInfoResult->distance < hitInfoNode->distance)
            {
                continue;
            }
        }

        if (currentNode.IsLeaf())
        {
            for (int index = currentNode.GetIndexBegin();
                 index < currentNode.GetIndexEnd();
                 index++)
            {
                const int primitiveID = m_primitiveIDs[index];

                const GeometryBase* const geometry =
                  scene.GetGeometries()[primitiveID];

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
            const int leftChildIndex = currentNode.GetChildNodes()[0];
            const int rightChildIndex = currentNode.GetChildNodes()[1];

            const float sqDistLeft =
              (m_bvhNodes[leftChildIndex].GetBounds().GetCenter() - ray.o)
                .SquaredLength();
            const float sqDistRight =
              (m_bvhNodes[rightChildIndex].GetBounds().GetCenter() - ray.o)
                .SquaredLength();

            if (sqDistLeft < sqDistRight)
            {
                bvhNodeIndexStack.emplace_back(rightChildIndex);
                bvhNodeIndexStack.emplace_back(leftChildIndex);
            }
            else
            {
                bvhNodeIndexStack.emplace_back(leftChildIndex);
                bvhNodeIndexStack.emplace_back(rightChildIndex);
            }
        }
    }
    return hitInfoResult;
}
} // namespace Core
} // namespace Petrichor
