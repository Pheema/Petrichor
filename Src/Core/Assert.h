#pragma once

#include <cassert>
#include <Ext/vdb/vdb.h>

#undef PHM_DEBUG
#ifdef PHM_DEBUG

#define ASSERT(x) assert(x)
#define UNIMPLEMENTED() assert(false && "This function is unimplemented")
#define IS_NORMALIZED(v) assert(Petrichor::Math::ApproxEq((v).Length(), 1.0f, Petrichor::kEps) && "'#v' is not normalized.")
#define IS_APPROXEQ(x, y) assert(Petrichor::Math::ApproxEq((x), (y), Petrichor::kEps) && "'#x' is not equal to '#y'.")

#define VDB_POINT(x, y, z) vdb_point((x), (y), (z))
#define VDB_POINT_VEC(v) vdb_point((v).x, (v).y, (v).z)
#define VDB_LINE_VEC(v0, v1) vdb_line((v0).x, (v0).y, (v0).z, (v1).x, (v1).y, (v1).z)
#define VDB_TRIANGLE_VEC(v0, v1, v2) vdb_triangle((v0).x, (v0).y, (v0).z, (v1).x, (v1).y, (v1).z, (v2).x, (v2).y, (v2).z)
#define VDB_SETCOLOR(color) vdb_color((color).x, (color).y, (color).z)

#define VDB_BOX(v0, v1) { \
const Petrichor::Math::Vector3f v000 = (v0);\
const Petrichor::Math::Vector3f v100 = Petrichor::Math::Vector3f((v1).x, (v0).y, (v0).z);\
const Petrichor::Math::Vector3f v010 = Petrichor::Math::Vector3f((v0).x, (v1).y, (v0).z);\
const Petrichor::Math::Vector3f v110 = Petrichor::Math::Vector3f((v1).x, (v1).y, (v0).z);\
const Petrichor::Math::Vector3f v001 = Petrichor::Math::Vector3f((v0).x, (v0).y, (v1).z);\
const Petrichor::Math::Vector3f v101 = Petrichor::Math::Vector3f((v1).x, (v0).y, (v1).z);\
const Petrichor::Math::Vector3f v011 = Petrichor::Math::Vector3f((v0).x, (v1).y, (v1).z);\
const Petrichor::Math::Vector3f v111 = (v1);\
VDB_LINE_VEC(v000, v010); VDB_LINE_VEC(v010, v110); VDB_LINE_VEC(v110, v100); VDB_LINE_VEC(v100, v000);\
VDB_LINE_VEC(v000, v001); VDB_LINE_VEC(v010, v011); VDB_LINE_VEC(v110, v111); VDB_LINE_VEC(v100, v101);\
VDB_LINE_VEC(v001, v011); VDB_LINE_VEC(v011, v111); VDB_LINE_VEC(v111, v101); VDB_LINE_VEC(v101, v001);\
}\

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
