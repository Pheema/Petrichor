#pragma once

#include "Core/Accel/Bound.h"
#include "Core/Material/MaterialBase.h"

namespace Petrichor
{
namespace Core
{

struct HitInfo;
struct Ray;
class ISampler2D;

struct PointData
{
    PointData()
      : pos()
      , normal()
    {
    }

    Math::Vector3f pos;
    Math::Vector3f normal;
};

class GeometryBase
{
public:
    // AABBの計算
    virtual Bound
    CalcBound() const
    {
        ASSERT(false);
        return Bound();
    }

    // レイとの交差判定
    virtual bool
    Intersect(const Ray& ray, HitInfo* hitInfo) const = 0;

    // ライト表面をサンプルリングする
    virtual void
    SampleSurface(Math::Vector3f p,
                  ISampler2D& sampler2D,
                  PointData* pointData,
                  float* pdfArea) const
    {
        *pdfArea = 0.0f;
        PointData pd;
        *pointData = pd;
        ASSERT(false);
    }

    // ジオメトリにマテリアルを設定
    inline void
    SetMaterial(const MaterialBase* material)
    {
        m_material = material;
    }

    // ジオメトリからマテリアルを取得
    inline const MaterialBase*
    GetMaterial(float randValue) const
    {
        const MaterialBase *mat0 = nullptr, *mat1 = nullptr;
        float mix = 0.0f;
        if (m_material->GetMaterialType(&mat0, &mat1, &mix) ==
            MaterialTypes::Mix)
        {
            if (randValue < mix)
            {
                return mat1;
            }
            else
            {
                return mat0;
            }
            ASSERT(false);
            return mat0;
        }

        return m_material;
    }

protected:
    // TODO: 苦肉の策
    // あとでCalcBound()とは別にSetBound()を作って分ける
    mutable Bound m_bound          = Bound();
    const MaterialBase* m_material = nullptr;
};

} // namespace Core
} // namespace Petrichor
