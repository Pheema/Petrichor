cmake_minimum_required(VERSION 3.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup()

add_library(LibPetrichor STATIC
	Core/Accel/Bounds.cpp
	Core/Accel/BruteForce.cpp
	Core/Accel/BVHNode.cpp
	Core/Accel/BinnedSAHBVH.cpp
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
	Core/Camera.cpp
	Core/Color3f.cpp
	Core/Environment.cpp
	Core/Petrichor.cpp
	Core/Scene.cpp
	Core/SceneSettings.cpp
	Core/Texture2D.cpp
	Core/TileManager.cpp
	Math/Halton.cpp
	Math/OrthonormalBasis.cpp
	Profiler/Profiler.cpp
	TestScene/TestScene.cpp
)

target_compile_definitions(
	LibPetrichor
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:STBI_MSC_SECURE_CRT> # [stb_image] Use secure CRT functions
		$<$<CONFIG:Debug>:PHM_DEBUG>
		$<$<CONFIG:RelWithDebInfo>:PHM_DEBUG>
)

target_compile_features(
	LibPetrichor
	PUBLIC
		cxx_std_17
)

target_compile_options(
	LibPetrichor
	PUBLIC
		$<$<CONFIG:Debug>:/MTd>
		$<$<CONFIG:RelWithDebInfo>:/MT>
		$<$<CONFIG:Release>:/MT>
)

target_include_directories(
	LibPetrichor
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)
conan_target_link_libraries(LibPetrichor)
