project(Petrichor)
cmake_minimum_required(VERSION 3.8)

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
  add_compile_options(/MP /utf-8)
endif(MSVC)

add_subdirectory(LibPetrichor)
add_subdirectory(Benchmark)
add_subdirectory(TestPetrichor)

add_executable(AppPetrichor AppPetrichor/AppPetrichor.cpp)

if(MSVC)
  target_compile_options(AppPetrichor
                         PRIVATE $<$<CONFIG:Debug>:/MTd>
                                 $<$<CONFIG:RelWithDebInfo>:/MT
                                 /fp:fast
                                 /arch:AVX2>
                                 $<$<CONFIG:Release>:/MT
                                 /GL
                                 /fp:fast
                                 /arch:AVX2>)
endif(MSVC)

target_include_directories(AppPetrichor PRIVATE LibPetrichor)

if(MSVC)
  set(linkFlagsRelease "/LTCG /OPT:REF /OPT:ICF")
endif(MSVC)

set_target_properties(
  AppPetrichor
  PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
             "${CMAKE_SOURCE_DIR}"
             VS_USER_PROPS
             "${CMAKE_BINARY_DIR}/conanbuildinfo_multi.props"
             LINK_FLAGS_RELEASE
             "${linkFlagsRelease}")

if(MSVC)
  target_link_libraries(AppPetrichor LibPetrichor CONAN_PKG::gflags)
else(MSVC)
  target_link_libraries(AppPetrichor LibPetrichor stdc++fs CONAN_PKG::gflags)
endif(MSVC)

# Startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT AppPetrichor)
