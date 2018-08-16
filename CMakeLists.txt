project(Petrichor)
cmake_minimum_required(VERSION 3.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
)

include_directories(LibPetrichor)
add_subdirectory(LibPetrichor)

add_executable(
    AppPetrichor
        AppPetrichor/AppPetrichor.cpp
)
set_target_properties(
    AppPetrichor
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
target_link_libraries(AppPetrichor LibPetrichor)
