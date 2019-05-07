cmake_minimum_required(VERSION 3.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup()

add_executable(Benchmark Math/BenchmarkVector3f.cpp)

target_compile_features(Benchmark PUBLIC cxx_std_17)

target_compile_options(Benchmark
                       PRIVATE $<$<CONFIG:Debug>:/MTd>
                               $<$<CONFIG:RelWithDebInfo>:/MT
                               /fp:fast
                               /arch:AVX2>
                               $<$<CONFIG:Release>:/MT
                               /fp:fast
                               /arch:AVX2>)

target_include_directories(Benchmark
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_SOURCE_DIR}/External
                                   ${CMAKE_SOURCE_DIR}/LibPetrichor)

conan_target_link_libraries(Benchmark)
