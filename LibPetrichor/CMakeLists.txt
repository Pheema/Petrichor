cmake_minimum_required(VERSION 3.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(
    /
    ${CONAN_INCLUDE_DIRS}
)

add_library(LibPetrichor STATIC
    Core/Accel/Bounds.cpp
    Core/Accel/BruteForce.cpp
    Core/Accel/BVHNode.cpp
    Core/Accel/SweepBVH.cpp
    Core/Geometry/Mesh.cpp
    Core/Geometry/Sphere.cpp
    Core/Geometry/Triangle.cpp
    Core/Geometry/Vertex.cpp
    Core/Integrator/PathTracing.cpp
    Core/Integrator/SimplePathTracing.cpp
    Core/Material/Emission.cpp
    Core/Material/GGX.cpp
    Core/Material/Lambert.cpp
    Core/Material/MatMix.cpp
    Core/Sampler/MicroJitteredSampler.cpp
    Core/Sampler/RandomSampler2D.cpp
    Core/Thread/ThreadPool.cpp
    Core/Camera.cpp
    Core/Environment.cpp
    Core/Petrichor.cpp
    Core/Scene.cpp
    Core/SceneSettings.cpp
    Core/Texture2D.cpp
    Core/TileManager.cpp
    Math/Halton.cpp
    Math/OrthonormalBasis.cpp
)

target_link_libraries(LibPetrichor ${CONAN_LIBS})