cmake_minimum_required(VERSION 3.14)

add_executable(TestPetrichor "Math/TestAliasMethod.cpp" "Math/TestVector3f.cpp"
                             "TestMain.cpp")

target_compile_features(TestPetrichor PUBLIC cxx_std_17)

target_include_directories(TestPetrichor
                           PRIVATE ${CMAKE_SOURCE_DIR}/LibPetrichor)

target_link_libraries(TestPetrichor CONAN_PKG::gtest LibPetrichor)
