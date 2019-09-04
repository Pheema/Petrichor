pushd .
cd ..

if not exist BuildPetrichor (
    mkdir BuildPetrichor
)

cd BuildPetrichor

conan install ../Petrichor -s arch=x86_64 -s build_type=Debug -s compiler.runtime=MTd --build=missing
conan install ../Petrichor -s arch=x86_64 -s build_type=RelWithDebInfo -s compiler.runtime=MT --build=missing
conan install ../Petrichor -s arch=x86_64 -s build_type=Release -s compiler.runtime=MT --build=missing
cmake ../Petrichor -G "Visual Studio 16 2019" -A x64
popd
