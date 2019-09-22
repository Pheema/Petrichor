cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0091 NEW)
project(Petrichor)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(MSVC)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
else(MSVC)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif(MSVC)
conan_basic_setup(TARGETS)

set(CMAKE_CONFIGURATION_TYPES
    "Debug;RelWithDebInfo;Release"
    CACHE STRING "" FORCE)

if(MSVC)
  add_compile_options(/MP
                      /utf-8
                      /arch:AVX2
                      /Oi
                      /Gy
                      $<$<CONFIG:Release>:/GL>)
  add_link_options($<$<CONFIG:Release>:/LTCG>)
endif(MSVC)

# ---- Open Image Denoise ----
option(OIDN_STATIC_LIB "Build Open Image Denoise as a static library." ON)
add_subdirectory(External/oidn)

add_subdirectory(LibPetrichor)
add_subdirectory(AppPetrichor)

option(PETRICHOR_BENCHMARK "" OFF)
if(PETRICHOR_BENCHMARK)
  # ---- Google benchmark ----
  option(BENCHMARK_ENABLE_TESTING "" OFF)
  option(BENCHMARK_ENABLE_INSTALL "" OFF)
  option(BENCHMARK_ENABLE_GTEST_TESTS "" OFF)
  add_subdirectory(External/benchmark)
  add_subdirectory(Benchmark)
endif()

option(PETRICHOR_TEST "" OFF)
if(PETRICHOR_TEST)
  add_subdirectory(TestPetrichor)
endif()

# Startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT AppPetrichor)
