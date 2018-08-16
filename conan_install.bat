if not exist Build (
    mkdir Build
)

pushd Build
conan install .. -g visual_studio_multi -s arch=x86_64 -s build_type=Debug -s compiler.runtime=MTd
conan install .. -g visual_studio_multi -s arch=x86_64 -s build_type=Release -s compiler.runtime=MT
cmake .. -G "Visual Studio 15 2017 Win64"
popd