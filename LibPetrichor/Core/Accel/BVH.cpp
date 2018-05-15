#include "BVH.h"

#include "Core/Accel/BVHNode.h"
#include "Core/Scene.h"
#include <stack>

namespace Petrichor {
namespace Core {

void
BVH::Build(const Scene& scene)
{
    m_bvhNodes.clear();

    {
        BVHNode rootNode;
        rootNode.ReserveChildArray(scene.GetGeometries().size());
        for (const auto* geometry : scene.GetGeometries()) {
            rootNode.bound.Expand(geometry->CalcBound());
            rootNode.AppendChild(geometry);
        }
        m_bvhNodes.emplace_back(rootNode);
    }

    // 処理すべきノードインデックスのスタック
    std::stack<size_t> bvhNodeIndexStack;
    bvhNodeIndexStack.emplace(0);

    size_t index = 0;
    size_t depth = 1;
    while (!bvhNodeIndexStack.empty()) {
        const size_t nodeIndex = bvhNodeIndexStack.top();
        bvhNodeIndexStack.pop();
        depth--;
        VDB_SETCOLOR(Color3f(0.5f * sin(3.87f * depth + 1.0f) + 0.5f,
                             0.5f * sin(11.5f * depth + 0.5f) + 0.5f,
                             0.5f * sin(7.71f * depth + 0.2f) + 0.5f));
        VDB_BOX(m_bvhNodes[nodeIndex].bound.vMin,
                m_bvhNodes[nodeIndex].bound.vMax);

        if (m_bvhNodes[nodeIndex].GetNumChildGeoms() < 16) {
            m_bvhNodes[nodeIndex].SetLeaf(true);
            continue;
        }

        int widestAxis = m_bvhNodes[nodeIndex].bound.GetWidestAxis();
        float center   = m_bvhNodes[nodeIndex].bound.Center()[widestAxis];
#if 0
        // 座標の中心で2分割する
        auto offsetMid = m_bvhNodes[nodeIndex].Partition(widestAxis);
        auto iterMid = m_bvhNodes[nodeIndex].GetChildArray().cbegin() + offsetMid;
#else
        // 一番長い辺に垂直に2分割
        // なるべく木の階層が浅くなるように均等な個数だけ入るようにする
        size_t halfSize = m_bvhNodes[nodeIndex].GetNumChildGeoms() / 2;
        m_bvhNodes[nodeIndex].SortByNthElement(widestAxis, center, halfSize);
        auto iterMid =
          m_bvhNodes[nodeIndex].GetChildArray().cbegin() + halfSize;
#endif

        BVHNode childNodes[2];

        if (iterMid == m_bvhNodes[nodeIndex].GetChildArray().cbegin()) {
            iterMid++;
        }

        if (iterMid == m_bvhNodes[nodeIndex].GetChildArray().cend()) {
            iterMid--;
        }

        const auto& iterBegin = m_bvhNodes[nodeIndex].GetChildArray().cbegin();
        const size_t numChildren0 = std::distance(iterBegin, iterMid);
        childNodes[0].ReserveChildArray(numChildren0);
        for (auto iter = iterBegin; iter != iterMid; ++iter) {
            childNodes[0].bound.Expand((*iter)->CalcBound());
            childNodes[0].AppendChild(*iter);
        }

        const auto& iterEnd = m_bvhNodes[nodeIndex].GetChildArray().cend();
        const size_t numChildren1 = std::distance(iterMid, iterEnd);
        childNodes[1].ReserveChildArray(numChildren1);
        for (auto iter = iterMid; iter != iterEnd; ++iter) {
            childNodes[1].bound.Expand((*iter)->CalcBound());
            childNodes[1].AppendChild(*iter);
        }

        // 子ノードをリストに登録
        m_bvhNodes.emplace_back(childNodes[0]);
        m_bvhNodes.emplace_back(childNodes[1]);

        // 子ノードを親ノードに登録
        const size_t childIndicies[2]{ ++index, ++index };
        m_bvhNodes[nodeIndex].SetChildNode(0, childIndicies[0]);
        m_bvhNodes[nodeIndex].SetChildNode(1, childIndicies[1]);

        bvhNodeIndexStack.emplace(childIndicies[0]);
        bvhNodeIndexStack.emplace(childIndicies[1]);
        depth += 2;

        m_bvhNodes[nodeIndex].ClearAndShrink();
    }
    std::cout << "[Done] BVH Construction" << std::endl;
}

bool
BVH::Intersect(const Ray& ray,
               HitInfo* hitInfo,
               float distMin,
               float distMax) const
{
    std::stack<size_t> bvhNodeIndexQueue;
    bvhNodeIndexQueue.emplace(0);

    // ---- BVHのトラバーサル ----
    bool isHit = false;
    HitInfo hitInfo_;

    // 2つの子ノード又は子オブジェクトに対して
    while (!bvhNodeIndexQueue.empty()) {
        // 葉ノードに対して
        const size_t bvhNodeIndex = bvhNodeIndexQueue.top();
        bvhNodeIndexQueue.pop();

        if (!m_bvhNodes[bvhNodeIndex].Intersect(ray, *hitInfo)) {
            // 衝突しない場合 or RayがAABBより手前で衝突している場合
            // 子ノードの探索はしない
            continue;
        }

        if (m_bvhNodes[bvhNodeIndex].IsLeaf()) {
            for (const auto* geometry :
                 m_bvhNodes[bvhNodeIndex].GetChildArray()) {
                if (geometry->Intersect(ray, &hitInfo_)) {
                    if (hitInfo_.distance < distMin ||
                        hitInfo_.distance > distMax) {
                        continue;
                    }

                    if (hitInfo_.distance < hitInfo->distance) {
                        isHit    = true;
                        *hitInfo = hitInfo_;
                    }
                }
            }
        }
        else {
            // 枝ノードの場合
            for (const auto childNodeIndex :
                 m_bvhNodes[bvhNodeIndex].GetChildNodeIndicies()) {
                bvhNodeIndexQueue.emplace(childNodeIndex);
            }
        }
    }
    return isHit;
}

} // namespace Core
} // namespace Petrichor
