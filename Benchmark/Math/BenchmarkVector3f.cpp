#include "Math/Vector3f.h"
#include "benchmark/benchmark.h"

using namespace Petrichor;

static constexpr int kNumLoops = 1000;

static void
BM_Vector3fAddTest(benchmark::State& state)
{
    Math::Vector3f v0(static_cast<float>(state.range(0)), 0.0f, 0.0f);
    Math::Vector3f v1(static_cast<float>(state.range(0)), 0.0f, 0.0f);
    for (auto _ : state)
    {
        for (int i = 0; i < kNumLoops; i++)
        {
            benchmark::DoNotOptimize(v0 + v1);
        }
    }
}
BENCHMARK(BM_Vector3fAddTest)->Arg(kNumLoops);

static void
BM_Vector3fDotTest(benchmark::State& state)
{
    Math::Vector3f v0(static_cast<float>(state.range(0)), 0.0f, 0.0f);
    Math::Vector3f v1(static_cast<float>(state.range(0)), 0.0f, 0.0f);
    for (auto _ : state)
    {
        for (int i = 0; i < kNumLoops; i++)
        {
            benchmark::DoNotOptimize(Math::Dot(v0, v1));
        }
    }
}
BENCHMARK(BM_Vector3fDotTest)->Arg(kNumLoops);

BENCHMARK_MAIN();
