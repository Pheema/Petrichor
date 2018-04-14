#include <Ext/gtest/gtest.h>

#include <Math/Vector3f.h>

// TODO: テストが雑

TEST(Dot, DotTest)
{
    using namespace Petrichor::Math;
    EXPECT_EQ(Dot(Vector3f(1.0f, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f)), 1.0f);
    EXPECT_EQ(Dot(Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f)), 1.0f);
    EXPECT_EQ(Dot(Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, 0.0f, 1.0f)), 1.0f);
    EXPECT_EQ(Dot(Vector3f(1.0f, 2.0f, 3.0f), Vector3f(4.0f, 5.0f, 6.0f)), 32.0f);
}

TEST(Cross, CrossTest)
{
    using namespace Petrichor::Math;
    Vector3f result0 = Cross(Vector3f::UnitX(), Vector3f::UnitY());
    EXPECT_TRUE(ApproxEq(result0, Vector3f::UnitZ()));
    Vector3f result1 = Cross(Vector3f::UnitY(), Vector3f::UnitZ());
    EXPECT_TRUE(ApproxEq(result1, Vector3f::UnitX()));
    Vector3f result2 = Cross(Vector3f::UnitZ(), Vector3f::UnitX());
    EXPECT_TRUE(ApproxEq(result2, Vector3f::UnitY()));
}

TEST(OperatorOverload, Addition)
{
    using namespace Petrichor::Math;
    // operator+
    EXPECT_TRUE(
        ApproxEq(
            Vector3f(1.0f, 2.0f, 3.0f) + Vector3f(4.0f, 5.0f, 6.0f),
            Vector3f(5.0f, 7.0f, 9.0f)
        )
    );
    EXPECT_TRUE(
        ApproxEq(
            Vector3f(-1.0f, -2.0f, -3.0f) + Vector3f(4.0f, 5.0f, 6.0f),
            Vector3f(3.0f, 3.0f, 3.0f)
        )
    );

    // operator+=
    Vector3f v0(1.0f, 2.0f, 3.0f);
    v0 += Vector3f(4.0f, 5.0f, 6.0f);
    EXPECT_TRUE(
        ApproxEq(v0, Vector3f(5.0f, 7.0f, 9.0f))
    );

    //unary operator+
    Vector3f v1(4.0f, 5.0f, 6.0f);
    v1 = +v1;
    EXPECT_TRUE(
        ApproxEq(v1, Vector3f(4.0f, 5.0f, 6.0f))
    );
}
