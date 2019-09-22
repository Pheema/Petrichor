#!/bin/bash
pushd .
cd ..

if [ ! -e BuildPetrichorLinux ]; then
    mkdir BuildPetrichorLinux
fi

cd BuildPetrichorLinux

conan install ../Petrichor/conanfile-linux.txt -pr ../Petrichor/conanprofile-clang.txt --build=missing
cmake ../Petrichor -G "Ninja" -DCMAKE_C_COMPILER=clang-7 -DCMAKE_CXX_COMPILER=clang++-7 -DTBB_ROOT=../tbb-linux -DLIBPETRICHOR_USE_INCLUDE_WHAT_YOU_USE=ON
ninja

popd
