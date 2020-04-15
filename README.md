# Visualization
A 3D visualizer for ns-3 scenarios.

## Requirements

* A C++ 17 compliant compiler
  * Minimum supported compilers:
    * GCC 7.3.0
    * Clang 6.0.0
* CMake 3.12 or greater
* A graphics card supporting OpenGL 3.3
* Qt 5.3

## Initial Setup

### Clone the Project
Clone the repository _recursively_ to collect the dependencies as well as the source

```shell
git clone --recursive {your ssh/https link}
```

Note: If you already cloned the project the normal way
You may pull the dependencies by running

```shell
git submodule update --recursive
```

### Building the Visualizer
Move into the project's root directory, and make a new directory called `build` and cd into it
```shell
mkdir build
cd build
```

Run `cmake` and specify the source directory. Then run the build tool
```shell
cmake ..
cmake --build . --parallel
```

### Running the Visualizer

#### Resource Paths
By default the Visualizer will search from the current working directory the following:

Textures in:
```shell
$(working_directory)/resources/textures
```

Models in:
```shell
$(working_directory)/resources/models
```
All searches for textures in models will be redirected to the texture source above

A solution independent of the current working directory is forthcoming...

#### Run the Program
After building the application may launched from the build directory with the following command:
```shell
./src/visualization
```

## Controls
Currently all controls are fixed. Perhaps in the future we may allow them to be changed.

### Camera
The camera may be rotated by clicking and holding the left mouse button on the scene.

The camera may be moved with the `W`, `A`, `S`, & `D` Keys:

`W`: Forward

`A`: Left

`S`: Backward

`D`: Right

### Play/Pause
The simulation starts in a paused state, the scene will still render, but time will not advance until it is unpaused.

`P`: Pause/Resume scenario playback.
