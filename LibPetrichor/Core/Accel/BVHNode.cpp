#include "BVHNode.h"

#include <Core/Constants.h>
#include <Core/Geometry/GeometryBase.h>
#include <Core/HitInfo.h>
#include <Core/Ray.h>

// #define USE_SIMD

#ifdef USE_SIMD
#include <immintrin.h>
#endif

namespace Petrichor
{
namespace Core
{

using namespace Math;

std::optional<HitInfo>
BVHNode::Intersect(const Ray& ray) const
{
#ifdef USE_SIMD
    __m128 o_ = _mm_loadu_ps(&ray.o.x);
    __m128 rayDir_ = _mm_loadu_ps(&ray.dir.x);
    __m128 vMin_ = _mm_loadu_ps(&GetBounds().vMin.x);
    __m128 vMax_ = _mm_loadu_ps(&GetBounds().vMax.x);

    rayDir_ = _mm_rcp_ps(rayDir_);

    __m128 t0_ = _mm_mul_ps(_mm_sub_ps(vMin_, o_), rayDir_);
    __m128 t1_ = _mm_mul_ps(_mm_sub_ps(vMax_, o_), rayDir_);

    __m128 tMin_ = _mm_min_ps(t0_, t1_);
    __m128 tMax_ = _mm_max_ps(t0_, t1_);

    o_ =
      _mm_max_ps(tMin_, _mm_shuffle_ps(tMin_, tMin_, _MM_SHUFFLE(3, 0, 2, 1)));
    rayDir_ =
      _mm_min_ps(tMax_, _mm_shuffle_ps(tMax_, tMax_, _MM_SHUFFLE(3, 0, 2, 1)));

    o_ = _mm_cmpgt_ps(o_, rayDir_);

    /*alignas(16) static float mask[4] = { 0xFFFF, 0xFFFF, 0xFFFF, 0x0 };
    __m128 mask_ = _mm_load_ps(mask);
    o_ = _mm_and_ps(o_, mask_);*/

    if (!_mm_testz_ps(o_, o_))
    {
        return std::nullopt;
    }

    alignas(16) static const uint32_t infs[4] = {
        0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF
    };

    __m128 infs_ = _mm_load_ps(reinterpret_cast<const float*>(&infs[0]));

    __m128 mask0_ = _mm_cmpgt_ps(t0_, _mm_setzero_ps());
    __m128 mask1_ = _mm_cmpgt_ps(t1_, _mm_setzero_ps());

    __m128 positiveT0_ = _mm_blendv_ps(infs_, t0_, mask0_);
    __m128 positiveT1_ = _mm_blendv_ps(infs_, t1_, mask1_);
    __m128 results_ = _mm_min_ps(positiveT0_, positiveT1_);

    alignas(16) static float results[4];
    _mm_store_ps(results, results_);

    float distance = std::min({ results[0], results[1], results[2] });

#else
    const Vector3f invRayDir = Vector3f::One() / ray.dir;
    const Vector3f t0 = (GetBounds().vMin - ray.o) * invRayDir;
    const Vector3f t1 = (GetBounds().vMax - ray.o) * invRayDir;

    constexpr uint8_t idxArray[] = { 0, 1, 2, 0 };

    for (uint8_t cnt = 0; cnt < 3; cnt++)
    {
        const uint8_t axis0 = idxArray[cnt];
        const uint8_t axis1 = idxArray[cnt + 1];

        if (ray.dir[axis0] == 0 || ray.dir[axis1] == 0)
        {
            // 軸に平行なレイが入ってきた場合
            continue;
        }

        const float tMinMax = std::max(std::min(t0[axis0], t1[axis0]),
                                       std::min(t0[axis1], t1[axis1]));
        const float tMaxMin = std::min(std::max(t0[axis0], t1[axis0]),
                                       std::max(t0[axis1], t1[axis1]));
        if (tMaxMin < tMinMax)
        {
            return std::nullopt;
        }
    }

    float distance = kInfinity;
    for (uint8_t axis = 0; axis < 3; ++axis)
    {
        if (t0[axis] > 0.0f)
        {
            distance = std::min(distance, t0[axis]);
        }

        if (t1[axis] > 0.0f)
        {
            distance = std::min(distance, t1[axis]);
        }
    }
#endif

    HitInfo hitInfo{ distance, nullptr };
    return hitInfo;
}

bool
BVHNode::Contains(const Math::Vector3f& point) const
{
    // #TODO: 判定怪しげ
    const Math::Vector3f diff = GetBounds().vMax - GetBounds().vMin;
    const Math::Vector3f point2 = point - GetBounds().vMin;

    if (diff.x < point2.x || diff.y < point2.y || diff.z < point2.z)
    {
        return false;
    }

    return true;
}

} // namespace Core
} // namespace Petrichor
