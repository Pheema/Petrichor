project(Petrichor)
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
)

add_subdirectory(LibPetrichor)

add_executable(
    AppPetrichor
    AppPetrichor/AppPetrichor.cpp
)

target_include_directories(
    AppPetrichor
    PRIVATE
        LibPetrichor
)

set_target_properties(
    AppPetrichor
    PROPERTIES
        VS_USER_PROPS "${CMAKE_BINARY_DIR}/conanbuildinfo_multi.props"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
target_link_libraries(
    AppPetrichor
    LibPetrichor
)
