cmake_minimum_required(VERSION 3.15)

add_executable(AppPetrichor AppPetrichor.cpp)
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
