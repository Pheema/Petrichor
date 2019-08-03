#include "Math/AliasMethod.h"
#include "Random/XorShift.h"
#include "gtest/gtest.h"
#include <random>
#include <vector>

namespace
{

using namespace Petrichor::Math;

class AliasMethodTest : public ::testing::Test
{
};

TEST_F(AliasMethodTest, ChiSquaredTest10)
{
    constexpr int kSeed = 36917234;
    std::mt19937 rng(kSeed);
    std::uniform_int_distribution<> uniformInt(0, 1000000);

    for (int testIndex = 0; testIndex < 1000; testIndex++)
    {
        std::vector<int> input;
        input.reserve(10);

        for (int i = 0; i < 10; i++)
        {
            input.emplace_back(uniformInt(rng));
        }

        std::vector<float> normalizedInput;
        {
            const int inputSum = std::accumulate(
              input.cbegin(), input.cend(), 0, [](int sum, int value) {
                  return sum + value;
              });
            normalizedInput.reserve(input.size());
            for (int elem : input)
            {
                normalizedInput.emplace_back(elem /
                                             static_cast<float>(inputSum));
            }
        }

        std::vector<int> freqs;
        freqs.resize(input.size());

        {
            AliasMethod aliasMethod;
            aliasMethod.Construct(input);
            XorShift128 rng0(13241);
            XorShift128 rng1(51234);

            constexpr int kNumSamples = 1000000;
            for (int i = 0; i < kNumSamples; i++)
            {
                const int index = aliasMethod.Sample(rng0.next(), rng1.next());
                freqs[index]++;
            }
        }

        std::vector<float> normalizedFreqs;
        {
            const int resultSum = std::accumulate(
              freqs.cbegin(), freqs.cend(), 0, [](int sum, int value) {
                  return sum + value;
              });

            normalizedFreqs.reserve(freqs.size());
            for (int elem : freqs)
            {
                normalizedFreqs.emplace_back(elem /
                                             static_cast<float>(resultSum));
            }
        }

        float squaredChi = 0.0f;
        for (int i = 0; i < static_cast<int>(input.size()); i++)
        {
            if (normalizedInput[i] == 0)
            {
                continue;
            }

            squaredChi += (normalizedInput[i] - normalizedFreqs[i]) *
                          (normalizedInput[i] - normalizedFreqs[i]) /
                          normalizedInput[i];
        }

        constexpr float kChiDeg9Sig099 = 2.09f;
        EXPECT_LE(squaredChi, kChiDeg9Sig099);
    }
}

} // namespace
