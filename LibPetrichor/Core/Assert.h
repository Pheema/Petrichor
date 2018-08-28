#pragma once

#include "Constants.h"

#ifdef PHM_DEBUG

#define ASSERT(expression)                                                     \
    if (expression)                                                            \
    {                                                                          \
        (void*)0;                                                              \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        int* tmp = nullptr;                                                    \
        *tmp = 0;                                                              \
    }

#define UNIMPLEMENTED() ASSERT(false && "This function is unimplemented")
#define IS_NORMALIZED(v)                                                       \
    ASSERT(Petrichor::Math::ApproxEq((v).Length(), 1.0f, Petrichor::kEps) &&   \
           "'#v' is not normalized.")
#define IS_APPROXEQ(x, y)                                                      \
    ASSERT(Petrichor::Math::ApproxEq((x), (y), Petrichor::kEps) &&             \
           "'#x' is not equal to '#y'.")

#else

#define ASSERT(x)
#define UNIMPLEMENTED()
#define IS_NORMALIZED(v)
#define IS_APPROXEQ(x, y)

#define VDB_POINT(v)
#define VDB_POINT_VEC(v)
#define VDB_LINE_VEC(v0, v1)
#define VDB_TRIANGLE_VEC(v0, v1, v2)
#define VDB_SETCOLOR(color)
#define VDB_BOX(v0, v1)

#endif
