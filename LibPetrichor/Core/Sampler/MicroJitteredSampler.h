#pragma once

#include "ISampler2D.h"
#include <Math/Halton.h>
#include <Random/XorShift.h>

namespace Petrichor
{
namespace Core
{

class MicroJitteredSampler : public ISampler2D
{
public:
    MicroJitteredSampler(unsigned seed);

    void Initialize(int lengthSeq);
    virtual std::tuple<float, float> SampleNext2D() final;

    unsigned GetIndex() const
    {
        return m_index;
    }

private:
    // TODO: スレッドごとに配列生成しててアホなので
    // 1個生成して使いまわす設計にする
    Math::Halton m_haltonBase2;
    Math::Halton m_haltonBase3;
    Math::XorShift128 m_xorShift;

    unsigned m_index;
    size_t m_lengthSeq;

    float m_mu;         // Jitteringさせる大きさ
    float m_offsetX, m_offsetY;
    const float kStarDiscrepancy = 2.5f;    // 論文で使用されていた値
};

}   // namespace Core
}   // namespace Petrichor
