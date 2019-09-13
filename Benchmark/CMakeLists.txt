cmake_minimum_required(VERSION 3.15)

add_executable(Benchmark Math/BenchmarkVector3f.cpp)

target_compile_features(Benchmark PRIVATE cxx_std_17)
target_include_directories(Benchmark PRIVATE ${CMAKE_SOURCE_DIR}/LibPetrichor)

if(MSVC)
  target_link_libraries(Benchmark
                        PRIVATE benchmark
                                LibPetrichor
                                benchmark_main
                                Shlwapi.lib)
else(MSVC)
  find_package(Threads)
  target_link_libraries(Benchmark
                        PRIVATE benchmark
                                benchmark_main
                                LibPetrichor
                                Threads::Threads)
endif(MSVC)
