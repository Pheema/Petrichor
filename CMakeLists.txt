project(Petrichor)
cmake_minimum_required(VERSION 3.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

if (MSVC_VERSION GREATER_EQUAL "1910")
    add_compile_options("/std:c++latest")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

include_directories(LibPetrichor)
add_subdirectory(LibPetrichor)

add_executable(AppPetrichor AppPetrichor/AppPetrichor.cpp)
set_target_properties(
    AppPetrichor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(AppPetrichor LibPetrichor)
