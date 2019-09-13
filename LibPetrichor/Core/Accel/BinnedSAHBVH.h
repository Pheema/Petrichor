#pragma once

#include "AccelBase.h"
#include "BVHNode.h"
#include "Core/Geometry/GeometryBase.h"
#include "Core/HitInfo.h"
#include <optional>

namespace Petrichor
{
namespace Core
{

class Scene;
struct Ray;

class BinnedSAHBVH : public AccelBase
{
private:
    struct PrimitiveData
    {
        int binID = -1;
        Bounds bounds;
        Math::Vector3f centroid;
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
               const BVHNode& currentNode,
               const std::vector<PrimitiveData>& primitiveDataArray,
               const std::vector<int>& primitiveIDs);

private:
    //! ビンの分割数
    constexpr static int kNumBins = 16;

    std::vector<PrimitiveData> m_primitiveData;
    std::vector<BVHNode> m_bvhNodes;
    std::vector<int> m_primitiveIDs;
};

} // namespace Core
} // namespace Petrichor
