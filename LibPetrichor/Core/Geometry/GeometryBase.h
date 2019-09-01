#pragma once

#include "COre/Material/MixMaterial.h"
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
    GetBounds() const = 0;

    // レイとの簡易交差判定
    virtual std::optional<HitInfo>
    Intersect(const Ray& ray) const = 0;

    // シェーディングに必要な情報をヒット情報から取得
    // (交差しないレイと図形の組み合わせに対して実行すると、正しい結果を得られないので注意)
    virtual ShadingInfo
    Interpolate(const Ray& ray, const HitInfo& hitInfo) const = 0;

    //! ジオメトリの重心を取得する
    //! @return 重心座標
    virtual Math::Vector3f
    GetCentroid() const = 0;

    // 表面をサンプルリングする
    virtual void
    SampleSurface(Math::Vector3f p,
                  ISampler2D& sampler2D,
                  PointData* pointData,
                  float* pdfArea) const = 0;

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
        if (m_material && m_material->GetMaterialType() == MaterialTypes::Mix)
        {
            auto mixMaterial = static_cast<const MixMaterial*>(m_material);
            return mixMaterial->GetSingleMaterial(randValue);
        }

        return m_material;
    }

protected:
    const MaterialBase* m_material = nullptr;
};

} // namespace Core
} // namespace Petrichor
