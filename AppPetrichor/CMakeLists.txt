cmake_minimum_required(VERSION 3.15)

add_executable(AppPetrichor AppPetrichor.cpp)

set_target_properties(
  AppPetrichor
  PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY
    "${CMAKE_BINARY_DIR}"
    VS_DEBUGGER_COMMAND_ARGUMENTS
    "--assetSetting ${CMAKE_SOURCE_DIR}/assets.json --renderSetting ${CMAKE_SOURCE_DIR}/settings.json"
    VS_USER_PROPS
    "${CMAKE_BINARY_DIR}/conanbuildinfo_multi.props"
    LINK_FLAGS_RELEASE
    "${linkFlagsRelease}")

target_include_directories(
  AppPetrichor
  PRIVATE LibPetrichor
          $<TARGET_PROPERTY:OpenImageDenoise,INTERFACE_INCLUDE_DIRECTORIES>)

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
