cmake_minimum_required(VERSION 3.15)
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
set(TBB_ROOT "${CMAKE_SOURCE_DIR}/../tbb")
add_subdirectory(External/oidn)

add_subdirectory(LibPetrichor)
add_subdirectory(Benchmark)
add_subdirectory(TestPetrichor)

add_executable(AppPetrichor AppPetrichor/AppPetrichor.cpp)

target_include_directories(AppPetrichor PRIVATE LibPetrichor)

set_target_properties(
  AppPetrichor
  PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
             "${CMAKE_SOURCE_DIR}"
             VS_USER_PROPS
             "${CMAKE_BINARY_DIR}/conanbuildinfo_multi.props"
             LINK_FLAGS_RELEASE
             "${linkFlagsRelease}")

if(MSVC)
  target_link_libraries(AppPetrichor
                        LibPetrichor
                        CONAN_PKG::gflags
                        CONAN_PKG::fmt
                        OpenImageDenoise)
else(MSVC)
  target_link_libraries(AppPetrichor
                        LibPetrichor
                        stdc++fs
                        CONAN_PKG::gflags
                        CONAN_PKG::fmt)
endif(MSVC)

# Startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT AppPetrichor)
