cmake_minimum_required(VERSION 3.14)

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
               Core/Logger.h
               Core/Logger.cpp
               Core/Petrichor.h
               Core/Petrichor.cpp
               Core/Ray.h
               Core/RenderSetting.h
               Core/RenderSettingLoader.h
               Core/RenderSettingLoader.cpp
               Core/Scene.h
               Core/Scene.cpp
               Core/SceneLoader.h
               Core/SceneLoader.cpp
               Core/Texture2D.h
               Core/Texture2D.cpp
               Core/TileManager.h
               Core/TileManager.cpp
               # Core/Accel/
               Core/Accel/AccelBase.h
               Core/Accel/BinnedSAHBVH.h
               Core/Accel/BinnedSAHBVH.cpp
               Core/Accel/BruteForce.h
               Core/Accel/BruteForce.cpp
               Core/Accel/AABB.h
               Core/Accel/AABB.cpp
               # Code/Denoiser/
               Core/Denoiser/IntelOpenImageDenoiser.h
               Core/Denoiser/IntelOpenImageDenoiser.cpp
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
               Core/Material/Glass.h
               Core/Material/Glass.cpp
               Core/Material/Lambert.h
               Core/Material/Lambert.cpp
               Core/Material/MaterialBase.h
               Core/Material/MixMaterial.h
               Core/Material/MixMaterial.cpp
               # Core/AOV/
               Core/AOV/AOVDenoisingNormal.h
               Core/AOV/AOVDenoisingNormal.cpp
               Core/AOV/AOVDenoisingAlbedo.h
               Core/AOV/AOVDenoisingAlbedo.cpp
               Core/AOV/AOVUVCoordinate.h
               Core/AOV/AOVUVCoordinate.cpp
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
               Math/AliasMethod.h
               Math/AliasMethod.cpp
               Math/Vector3f.h
               Math/MathUtils.h
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

target_include_directories(LibPetrichor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(
  LibPetrichor SYSTEM
  PRIVATE ${CMAKE_SOURCE_DIR}/External/stb
          ${CMAKE_SOURCE_DIR}/External/tinytoml/include
          $<TARGET_PROPERTY:OpenImageDenoise,INTERFACE_INCLUDE_DIRECTORIES>
  PUBLIC ${CMAKE_SOURCE_DIR}/External/spdlog/include)

target_link_libraries(LibPetrichor
                      PRIVATE OpenImageDenoise
                              CONAN_PKG::Assimp
                              CONAN_PKG::jsonformoderncpp
                              CONAN_PKG::fmt)

option(LIBPETRICHOR_USE_INCLUDE_WHAT_YOU_USE
       "Use include-what-you-use in build." OFF)
if(LIBPETRICHOR_USE_INCLUDE_WHAT_YOU_USE)
  find_program(IWYU_PATH NAMES include-what-you-use iwyu)
  if(IWYU_PATH)
    set_target_properties(LibPetrichor
                          PROPERTIES CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
  else()
    message(WARNING "Include-what-you-use not found.")
  endif()
endif()
