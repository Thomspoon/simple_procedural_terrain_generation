# Procedural Landscape Generation

This project demonstrates simple landscape generation using Perlin noise. The end-result allows the user to tweak values such as lacunarity, octave count, and persistence at run-time and see the result.

# Building

To build from source the following are required:

 - C++17 compiler
 - CMake 3.2+

First clone the repository:

```
git clone https://github.com/Thomspoon/simple_procedural_terrain_generation.git
```
Next, pull in the submodules

```
cd procedural_terrain_generation
git submodule update --init --recursive
```
Finally, create a build folder and run cmake (with whatever compiler you want as long as it supports C++17):

```
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-9 -DCMAKE_C_COMPILER=gcc-9
```

# Running

The program should be available under the `src` folder in the build directory.
