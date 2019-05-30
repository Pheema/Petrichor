cmake_minimum_required(VERSION 3.8)

add_executable(Benchmark "Math/BenchmarkVector3f.cpp")

target_compile_features(Benchmark PRIVATE cxx_std_17)
target_include_directories(Benchmark PRIVATE ${CMAKE_SOURCE_DIR}/LibPetrichor)

if(MSVC)
  target_compile_options(Benchmark
                         PRIVATE $<$<CONFIG:Debug>:/MTd>
                                 $<$<CONFIG:RelWithDebInfo>:/MT
                                 /fp:fast
                                 /arch:AVX2>
                                 $<$<CONFIG:Release>:/MT
                                 /GL
                                 /fp:fast
                                 /arch:AVX2>)
endif(MSVC)

if(MSVC)
  target_link_libraries(Benchmark
                        PRIVATE CONAN_PKG::google-benchmark Shlwapi.lib)
else(MSVC)
  find_package(Threads)
  target_link_libraries(Benchmark
                        PRIVATE Threads::Threads CONAN_PKG::google-benchmark)
endif(MSVC)
