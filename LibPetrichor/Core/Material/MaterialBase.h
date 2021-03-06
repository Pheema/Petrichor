#pragma once

#include "Core/Color3f.h"
#include "Core/HitInfo.h"

namespace Petrichor
{
namespace Core
{

struct Ray;
struct HitInfo;
class ISampler2D;

enum class MaterialTypes
{
    Lambert,
    Glass,
    Emission,
    Glossy,
    Mix
};

class MaterialBase
{
public:
    MaterialBase() = default;
    virtual ~MaterialBase() {}

    virtual Color3f
    BxDF(const Ray& rayIn,
         const Ray& rayOut,
         const ShadingInfo& shadingInfo) const = 0;

    virtual float
    PDF(const Ray& rayIn,
        const Ray& rayOut,
        const ShadingInfo& shadingInfo) const
    {
        ASSERT(false);
        return 0.0f;
    };

    virtual Ray
    CreateNextRay(const Ray& rayIn,
                  const ShadingInfo& shadingInfo,
                  ISampler2D& sampler2D) const = 0;

    virtual MaterialTypes
    GetMaterialType() const = 0;

    void
    EnableImportanceSampling(bool isEnabled)
    {
        m_isImportanceSamplingEnabled = isEnabled;
    }

    bool
    IsImportanceSamplingEnabled() const
    {
        return m_isImportanceSamplingEnabled;
    }

private:
    bool m_isImportanceSamplingEnabled = true;
};

} // namespace Core
} // namespace Petrichor
