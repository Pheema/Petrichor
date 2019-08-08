cmake_minimum_required(VERSION 3.15)

add_executable(TestPetrichor "Math/TestVector3f.cpp" "TestMain.cpp" "Math/TestAliasMethod.cpp")

target_compile_features(TestPetrichor PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(TestPetrichor
                         PRIVATE $<$<CONFIG:RelWithDebInfo>:
                                 /fp:fast
                                 /arch:AVX2>
                                 $<$<CONFIG:Release>:
                                 /fp:fast
                                 /arch:AVX2>)
endif(MSVC)

target_include_directories(TestPetrichor
                           PRIVATE ${CMAKE_SOURCE_DIR}/LibPetrichor)

target_link_libraries(TestPetrichor CONAN_PKG::gtest LibPetrichor)
