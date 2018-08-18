#pragma once

#include "Core/Accel/Bounds.h"
#include "Core/HitInfo.h"
#include "Core/Material/MaterialBase.h"
#include <optional>

namespace Petrichor
{
namespace Core
{

struct Ray;
class ISampler2D;

struct PointData
{
    Math::Vector3f pos;
    Math::Vector3f normal;
};

class GeometryBase
{
public:
    // AABBの計算
    virtual Bounds
    CalcBound() const
    {
        ASSERT(false);
        return Bounds{};
    }

    // レイとの簡易交差判定
    virtual std::optional<HitInfo>
    Intersect(const Ray& ray) const = 0;

    // シェーディングに必要な情報をヒット情報から取得
    // (交差しないレイと図形の組み合わせに対して実行すると、正しい結果を得られないので注意)
    virtual ShadingInfo
    Interpolate(const Ray& ray, const HitInfo& hitInfo) const
    {
        throw std::logic_error("No impl.");
    }

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
            return mat0;
        }

        return m_material;
    }

protected:
    const MaterialBase* m_material = nullptr;
};

} // namespace Core
} // namespace Petrichor
