cmake_minimum_required(VERSION 3.8)

if(MSVC)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
else(MSVC)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif(MSVC)
conan_basic_setup()

add_library(LibPetrichor STATIC "")

target_sources(LibPetrichor PRIVATE
               # Core/
               Core/Assert.h
               Core/Camera.h
               Core/Camera.cpp
               Core/Color3f.h
               Core/Color3f.cpp
               Core/Environment.h
               Core/Environment.cpp
               Core/HitInfo.h
               Core/Petrichor.h
               Core/Petrichor.cpp
               Core/Ray.h
               Core/RenderSetting.h
               Core/RenderSettingLoader.h
               Core/RenderSettingLoader.cpp
               Core/Scene.h
               Core/Scene.cpp
               Core/Texture2D.h
               Core/Texture2D.cpp
               Core/TileManager.h
               Core/TileManager.cpp
               # Core/Accel/
               Core/Accel/AccelBase.h
               Core/Accel/BinnedSAHBVH.h
               Core/Accel/BinnedSAHBVH.cpp
               Core/Accel/Bounds.h
               Core/Accel/Bounds.cpp
               Core/Accel/BruteForce.h
               Core/Accel/BruteForce.cpp
               Core/Accel/BVHNode.h
               Core/Accel/BVHNode.cpp
               # Core/Geometry/
               Core/Geometry/GeometryBase.h
               Core/Geometry/Mesh.h
               Core/Geometry/Mesh.cpp
               Core/Geometry/Sphere.h
               Core/Geometry/Sphere.cpp
               Core/Geometry/Triangle.h
               Core/Geometry/Triangle.cpp
               Core/Geometry/Vertex.h
               Core/Geometry/Vertex.cpp
               # Core/Integrator/
               Core/Integrator/PathTracing.h
               Core/Integrator/PathTracing.cpp
               Core/Integrator/SimplePathTracing.h
               Core/Integrator/SimplePathTracing.cpp
               # Core/Material/
               Core/Material/Emission.h
               Core/Material/Emission.cpp
               Core/Material/GGX.h
               Core/Material/GGX.cpp
               Core/Material/Lambert.h
               Core/Material/Lambert.cpp
               Core/Material/MaterialBase.h
               Core/Material/MatMix.h
               Core/Material/MatMix.cpp
               # Core/Sampler/
               Core/Sampler/ISampler1D.h
               Core/Sampler/ISampler2D.h
               Core/Sampler/MicroJitteredSampler.h
               Core/Sampler/MicroJitteredSampler.cpp
               Core/Sampler/RandomSampler1D.h
               Core/Sampler/RandomSampler2D.h
               Core/Sampler/RandomSampler2D.cpp
               # Core/Thread/
               Core/Thread/ThreadPool.h Core/Thread/ThreadPool.cpp
               # Math/
               Math/Halton.h
               Math/Halton.cpp
               Math/OrthonormalBasis.h
               Math/OrthonormalBasis.cpp
               # Profiler/
               Profiler/Profiler.h Profiler/Profiler.cpp
               # Random/
               Random/XorShift.h
               # TestScene/
               TestScene/TestScene.h TestScene/TestScene.cpp)

target_compile_definitions(
  LibPetrichor
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:STBI_MSC_SECURE_CRT> # [stb_image] Use secure CRT
                                                 # functions
  $<$<CONFIG:Debug>:PHM_DEBUG>
  $<$<CONFIG:RelWithDebInfo>:PHM_DEBUG>)

target_compile_features(LibPetrichor PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(LibPetrichor
                         PRIVATE $<$<CONFIG:Debug>:/MTd>
                                 $<$<CONFIG:RelWithDebInfo>:/MT
                                 /fp:fast
                                 /arch:AVX2>
                                 $<$<CONFIG:Release>:/MT
                                 /fp:fast
                                 /arch:AVX2>)
else(MSVC)
  target_compile_options(LibPetrichor INTERFACE "-lstdc++fs")
endif(MSVC)

target_include_directories(LibPetrichor
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_SOURCE_DIR}/External)

conan_target_link_libraries(LibPetrichor)
