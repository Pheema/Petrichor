project(Petrichor)
cmake_minimum_required(VERSION 3.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup()

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release"
    CACHE STRING ""
    FORCE)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
add_compile_options($<$<C_COMPILER_ID:MSVC>:/utf-8>)

add_subdirectory(LibPetrichor)
add_subdirectory(TestPetrichor)
add_subdirectory(Benchmark)

add_executable(AppPetrichor AppPetrichor/AppPetrichor.cpp)

target_compile_options(AppPetrichor
                       PRIVATE $<$<CONFIG:Debug>:/MTd>
                               $<$<CONFIG:RelWithDebInfo>:/MT
                               /fp:fast
                               /arch:AVX2>
                               $<$<CONFIG:Release>:/MT
                               /GL
                               /fp:fast
                               /arch:AVX2>)

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

target_link_libraries(AppPetrichor LibPetrichor)

conan_target_link_libraries(AppPetrichor)

# Set the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT AppPetrichor)
