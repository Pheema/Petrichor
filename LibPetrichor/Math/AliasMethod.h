#pragma once

#include "Core/Assert.h"
#include <algorithm>
#include <numeric>
#include <stack>
#include <vector>

namespace Petrichor
{
namespace Math
{

class AliasMethod
{
private:
    struct BoxColumn
    {
        int lowerIndex = -1;
        int upperIndex = -1;
        float threshold = 0.0f;
    };

public:
    //! ボックスの構築
    template<class T>
    void
    Construct(const std::vector<T>& distribution)
    {
        static_assert(std::is_arithmetic<T>::value);

        // 入力された分布のビン
        struct Bin
        {
            Bin(int index, float value)
              : index(index)
              , value(value)
            {
            }

            int index = -1;
            float value = 0.0f;
        };

        const float sum =
          std::accumulate(distribution.cbegin(),
                          distribution.cend(),
                          0.0f,
                          [](const float sum, const T value) {
                              return sum + static_cast<float>(value);
                          });

        const float average = sum / distribution.size();

        std::stack<Bin> tallBins;
        std::stack<Bin> shortBins;

        for (int index = 0; index < static_cast<int>(distribution.size());
             index++)
        {
            const float value = distribution[index] / average;

            if (1.0f <= value)
            {
                tallBins.emplace(index, value);
            }
            else
            {
                shortBins.emplace(index, value);
            }
        }

        m_boxColumns.resize(distribution.size());

        while (!shortBins.empty())
        {
            const Bin shortBin = shortBins.top();
            shortBins.pop();
            m_boxColumns[shortBin.index].lowerIndex = shortBin.index;
            m_boxColumns[shortBin.index].threshold =
              std::clamp(shortBin.value, 0.0f, 1.0f);

            if (!tallBins.empty())
            {
                // 足りない分を大きいカラムから拝借
                {
                    Bin& tallBin = tallBins.top();
                    m_boxColumns[shortBin.index].upperIndex = tallBin.index;

                    tallBin.value -=
                      std::clamp(1.0f - m_boxColumns[shortBin.index].threshold,
                                 0.0f,
                                 1.0f);
                }

                // tallColumnsのtopが平均より短くなっていたらshortColumnsに移動
                if (tallBins.top().value <= 1.0f ||
                    (shortBins.empty() && !tallBins.empty()))
                {
                    shortBins.emplace(tallBins.top());
                    tallBins.pop();
                }
            }
        }
    }

    //! 構築時に指定された分布に応じた比率でインデックスを取得する
    //! @param r0 [0, 1)の一様乱数
    //! @param r1 [0, 1)の一様乱数
    int
    Sample(float r0, float r1) const;

private:
    std::vector<BoxColumn> m_boxColumns;
};

} // namespace Math
} // namespace Petrichor
