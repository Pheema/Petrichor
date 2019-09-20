#pragma once

#include "AABB.h"
#include "AccelBase.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/HitInfo.h"
#include <array>
#include <optional>
#include <vector>

namespace Petrichor
{
namespace Core
{

class Scene;
struct Ray;

class BinnedSAHBVH : public AccelBase
{
private:
    //! BVH-node
    struct Node
    {
        Node() = default;

        Node(const AABB& boundary, int primIndexBegin, int prinIndexEnd)
          : boundary(boundary)
          , primIndexBegin(primIndexBegin)
          , primIndexEnd(prinIndexEnd)
        {
        }

        Node(const AABB& boundary,
             const std::array<int, 2>& childIndicies,
             int primIndexBegin,
             int primIndexEnd,
             bool isLeaf)
          : boundary(boundary)
          , childIndicies(childIndicies)
          , primIndexBegin(primIndexBegin)
          , primIndexEnd(primIndexEnd)
          , isLeaf(isLeaf)
        {
        }

        AABB boundary{};
        std::array<int, 2> childIndicies{ 0, 0 };
        int primIndexBegin = 0;
        int primIndexEnd = 0;
        bool isLeaf = false;
    };

    struct PrimitiveData
    {
        PrimitiveData() = default;

        PrimitiveData(int binID,
                      const AABB& boundary,
                      const Math::Vector3f& centroid)
          : binID(binID)
          , boundary(boundary)
          , centroid(centroid)
        {
        }

        int binID = -1;
        AABB boundary{};
        Math::Vector3f centroid{};
    };

public:
    BinnedSAHBVH() = default;

    void
    Build(const Scene& scene) override;

    std::optional<HitInfo>
    Intersect(const Ray& ray,
              const Scene& scene,
              float distMin,
              float distMax) const override;

private:
    //! @param binPartitionIndex どのビン番号でノードを左右に分割するか
    //! @param primitiveDataArray
    //! @param primitiveIDs
    //! @return [コスト, 左のノードに含まれるプリミティブ数]
    static std::pair<float, int>
    GetSAHCost(int binPartitionIndex,
               const Node& currentNode,
               const std::vector<PrimitiveData>& primitiveDataArray,
               const std::vector<int>& primitiveIDs);

    static std::optional<HitInfo>
    Intersect(const Ray& ray,
              const AABB& aabb,
              const PrecalcedData& preCalcedData)
    {
        const auto& sign = preCalcedData.sign;
        const auto& invRayDir = preCalcedData.invRayDir;

        float tMin = (aabb[sign[0]].x - ray.o.x) * invRayDir.x;
        float tMax = (aabb[1 - sign[0]].x - ray.o.x) * invRayDir.x;

        {
            const float tyMin = (aabb[sign[1]].y - ray.o.y) * invRayDir.y;
            const float tyMax = (aabb[1 - sign[1]].y - ray.o.y) * invRayDir.y;

            if (tyMax < tMin || tMax < tyMin)
            {
                return std::nullopt;
            }

            if (tMin < tyMin)
            {
                tMin = tyMin;
            }

            if (tyMax < tMax)
            {
                tMax = tyMax;
            }
        }

        {
            const float tzMin = (aabb[sign[2]].z - ray.o.z) * invRayDir.z;
            const float tzMax = (aabb[1 - sign[2]].z - ray.o.z) * invRayDir.z;

            if (tzMax < tMin || tMax < tzMin)
            {
                return std::nullopt;
            }

            if (tMin < tzMin)
            {
                tMin = tzMin;
            }

            if (tzMax < tMax)
            {
                tMax = tzMax;
            }
        }

        float distance = std::numeric_limits<float>::max();

        const bool isNearCollisionValid = (0.0f <= tMin && tMin < distance);
        if (isNearCollisionValid)
        {
            distance = tMin;
        }

        const bool isFarCollisionValid = (0.0f <= tMax && tMax < distance);
        if (isFarCollisionValid)
        {
            distance = tMax;
        }

        if (!(isNearCollisionValid || isFarCollisionValid))
        {
            return std::nullopt;
        }

        return [&] {
            HitInfo h;
            h.distance = distance;
            return h;
        }();
    }

private:
    //! ビンの分割数
    static constexpr int kNumBins = 16;

    std::vector<PrimitiveData> m_primitiveData;
    std::vector<Node> m_nodes;
    std::vector<int> m_primitiveIDs;
    int m_maxBVHDepth = 0;
};

} // namespace Core
} // namespace Petrichor
