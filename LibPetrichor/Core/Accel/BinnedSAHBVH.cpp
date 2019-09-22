#include "BinnedSAHBVH.h"

#include "Core/Geometry/GeometryBase.h"
#include "Core/Logger.h"
#include "Core/Scene.h"
#include <numeric>
#include <stack>
#include <tuple>

namespace Petrichor
{
namespace Core
{

void
BinnedSAHBVH::Build(const Scene& scene)
{
    SCOPE_LOGGER("[BVH] Build");

    const auto numPrimitives = scene.GetGeometries().size();
    m_maxBVHDepth = 0;
    int bvhDepth = 0;

    // 事前に全プリミティブのバウンディングボックスと重心を計算しておく
    {
        m_primitiveData.clear();
        m_primitiveData.reserve(numPrimitives);
        for (const GeometryBase* primitive : scene.GetGeometries())
        {
            const AABB boundary = primitive->CalcBoundary();
            const Math::Vector3f centroid = primitive->GetCentroid();
            m_primitiveData.emplace_back(-1, boundary, centroid);
        }
        m_primitiveData.shrink_to_fit();
    }

    std::stack<uint32_t> nodeIndexStack;

    // ルートのノードを計算
    {
        m_nodes.clear();
        m_nodes.reserve(2ll * numPrimitives);

        const AABB rootNodeBoundary = [&] {
            AABB boundary;
            for (const auto* primitive : scene.GetGeometries())
            {
                boundary.Merge(primitive->CalcBoundary());
            }
            return boundary;
        }();

        m_nodes.emplace_back(
          rootNodeBoundary, 0, static_cast<uint32_t>(numPrimitives));
        nodeIndexStack.emplace(0); // root
        bvhDepth++;
    }

    // InPlaceソート用プリミティブID配列を初期化
    {
        m_primitiveIDs.clear();
        m_primitiveIDs.resize(numPrimitives);
        std::iota(std::begin(m_primitiveIDs), std::end(m_primitiveIDs), 0);
        m_primitiveIDs.shrink_to_fit();
    }

    while (!nodeIndexStack.empty())
    {
        const uint32_t currentNodeIndex = nodeIndexStack.top();
        nodeIndexStack.pop();
        Node& currentNode = m_nodes[currentNodeIndex];

        const int indexBegin = currentNode.primIndexBegin;
        const int indexEnd = currentNode.primIndexEnd;
        ASSERT(indexBegin <= indexEnd);
        const int numPrimitivesInCurrentNode = (indexEnd - indexBegin);

        // ノード内のプリミティブ数が十分に少ない場合は分割しない
        constexpr int kMinNumPrimitivesInNode = 4;
        if (numPrimitivesInCurrentNode <= kMinNumPrimitivesInNode)
        {
            currentNode.isLeaf = true;
            bvhDepth--;
            continue;
        }

        // バウンディングボックスの一番長い辺に沿ってソート
        const auto iterBegin = std::begin(m_primitiveIDs) + indexBegin;
        const auto iterEnd = std::begin(m_primitiveIDs) + indexEnd;

        // ビンをアップデート
        const AABB binBoundary = [&] {
            AABB aabb;
            for (auto iter = iterBegin; iter != iterEnd; iter++)
            {
                const int primitiveID = *iter;
                aabb.Merge(m_primitiveData[primitiveID].centroid);
            }
            return aabb;
        }();

        const int widestAxis = binBoundary.GetWidestAxis();

        std::sort(iterBegin, iterEnd, [this, widestAxis](int id0, int id1) {
            return m_primitiveData[id0].centroid[widestAxis] <
                   m_primitiveData[id1].centroid[widestAxis];
        });

        // どのビンに属しているかを番号付け
        const float widestEdgeLength =
          binBoundary.upper[widestAxis] - binBoundary.lower[widestAxis];

        for (auto iter = iterBegin; iter != iterEnd; iter++)
        {
            const int primitiveID = *iter;
            PrimitiveData& primitiveData = m_primitiveData[primitiveID];

            const float l = primitiveData.centroid[widestAxis] -
                            binBoundary.lower[widestAxis];

            const auto binID = static_cast<int>(
              std::nextafter(kNumBins * l / widestEdgeLength, 0.0f));
            ASSERT(0 <= binID && binID < kNumBins);
            primitiveData.binID = binID;
        }

        // 最適な分割位置を探索
        const auto [binPartitionIndexInBestDiv,
                    numPrimsInLeftInBestDiv] = [&]() -> std::pair<int, int> {
            int binPartitionIndexInBestDiv_ = 0;
            int numPrimsInLeftInBestDiv_ = 0;
            float minCost = std::numeric_limits<float>::max();
            for (int binPartitionIndex = 0; binPartitionIndex < kNumBins;
                 binPartitionIndex++)
            {
                const auto [cost, numPrimsInLeft] =
                  GetSAHCost(binPartitionIndex,
                             currentNode,
                             m_primitiveData,
                             m_primitiveIDs);

                if (cost < minCost)
                {
                    minCost = cost;
                    binPartitionIndexInBestDiv_ = binPartitionIndex;
                    numPrimsInLeftInBestDiv_ = numPrimsInLeft;
                }
            }

            return { binPartitionIndexInBestDiv_, numPrimsInLeftInBestDiv_ };
        }();

        if (binPartitionIndexInBestDiv == 0)
        {
            bvhDepth--;
            currentNode.isLeaf = true;
            continue;
        }

        bvhDepth++;
        m_maxBVHDepth = std::max(m_maxBVHDepth, bvhDepth);

        // ---- 分割する場合 ----
        {
            const auto [leftBoundary, rightBoundary] =
              [&,
               binPartitionIndexInBestDiv =
                 binPartitionIndexInBestDiv]() -> std::pair<AABB, AABB> {
                AABB leftBoundary_, rightBoundary_;
                for (auto iter = iterBegin; iter != iterEnd; iter++)
                {
                    const int primitiveID = *iter;
                    const PrimitiveData& primitiveData =
                      m_primitiveData[primitiveID];

                    const bool isInLeftNode =
                      (primitiveData.binID < binPartitionIndexInBestDiv);
                    if (isInLeftNode)
                    {
                        leftBoundary_.Merge(primitiveData.boundary);
                    }
                    else
                    {
                        rightBoundary_.Merge(primitiveData.boundary);
                    }
                }

                return { leftBoundary_, rightBoundary_ };
            }();

            // Left child node
            {
                int leftChildIndex = 0;
                int rightChildIndex = 0;

                {
                    m_nodes.emplace_back(leftBoundary,
                                         std::array{ -1, -1 },
                                         currentNode.primIndexBegin,
                                         currentNode.primIndexBegin +
                                           numPrimsInLeftInBestDiv,
                                         false);

                    const auto index = static_cast<int>(m_nodes.size() - 1);
                    nodeIndexStack.emplace(index);
                    leftChildIndex = index;
                }

                {
                    m_nodes.emplace_back(rightBoundary,
                                         std::array{ -1, -1 },
                                         currentNode.primIndexBegin +
                                           numPrimsInLeftInBestDiv,
                                         currentNode.primIndexEnd,
                                         false);
                    const auto index = static_cast<int>(m_nodes.size() - 1);
                    nodeIndexStack.emplace(index);
                    rightChildIndex = index;
                }

                currentNode.childIndicies = { leftChildIndex, rightChildIndex };
            }
        }
    }
}

std::pair<float, int>
BinnedSAHBVH::GetSAHCost(int binPartitionIndex,
                         const Node& currentNode,
                         const std::vector<PrimitiveData>& primitiveDataArray,
                         const std::vector<int>& primitiveIDs)
{
    AABB leftBounds, rightBounds;
    int numPrimsInLeft = 0, numPrimsInRight = 0;

    const auto iterBegin =
      std::begin(primitiveIDs) + currentNode.primIndexBegin;
    const auto iterEnd = std::begin(primitiveIDs) + currentNode.primIndexEnd;

    for (auto iter = iterBegin; iter != iterEnd; iter++)
    {
        const int primitiveID = *iter;

        const PrimitiveData* primitiveData = &primitiveDataArray[primitiveID];

        const int primitiveBinID = primitiveData->binID;
        if (primitiveBinID < binPartitionIndex)
        {
            // left
            leftBounds.Merge(primitiveData->boundary);
            numPrimsInLeft++;
        }
        else
        {
            // right
            rightBounds.Merge(primitiveData->boundary);
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
    bvhNodeIndexStack.reserve(m_maxBVHDepth);
    bvhNodeIndexStack.emplace_back(0);

    // ---- BVHのトラバーサル ----
    std::optional<HitInfo> hitInfoResult;

    // 2つの子ノード又は子オブジェクトに対して
    while (!bvhNodeIndexStack.empty())
    {
        // 葉ノードに対して
        const int currentNodeIndex = bvhNodeIndexStack.back();
        bvhNodeIndexStack.pop_back();
        const Node& currentNode = m_nodes[currentNodeIndex];

        const auto hitInfoNode =
          Intersect(ray, currentNode.boundary, precalced);

        // そもそもBVHノードに当たる軌道ではない
        if (!hitInfoNode)
        {
            continue;
        }

        // 自ノードより手前で既に衝突している
        if (hitInfoResult && currentNode.boundary.Contains(ray.o) == false)
        {
            if (hitInfoResult->distance < hitInfoNode->distance)
            {
                continue;
            }
        }

        if (currentNode.isLeaf)
        {
            for (int index = currentNode.primIndexBegin;
                 index < currentNode.primIndexEnd;
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
            const int leftChildIndex = currentNode.childIndicies[0];
            const int rightChildIndex = currentNode.childIndicies[1];

            const float sqDistLeft =
              (m_nodes[leftChildIndex].boundary.CalcCentroid() - ray.o)
                .SquaredLength();
            const float sqDistRight =
              (m_nodes[rightChildIndex].boundary.CalcCentroid() - ray.o)
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
