#include "Math/Vector3f.h"
#include "gtest/gtest.h"

namespace
{

using namespace Petrichor::Math;

class Vector3fTest : public ::testing::Test
{
};

TEST_F(Vector3fTest, OperatorAdd)
{
    Vector3f v0{ 1.0f, 2.0f, 3.0f };
    Vector3f v1{ 4.0f, 5.0f, 6.0f };
    Vector3f v2{ 5.0f, 7.0f, 9.0f };

    const auto added = v0 + v1;
    for (int i = 0; i < 3; i++)
    {
        EXPECT_FLOAT_EQ(added[i], v2[i]);
    }
}

} // namespace
