# Visualization
A 3D visualizer for ns-3 scenarios.

## Requirements

* A C++ 17 compliant compiler
  * Minimum supported compilers:
    * GCC 7.3.0
    * Clang 6.0.0
* CMake 3.13 or greater
* A graphics card supporting OpenGL 3.3
* Qt 5.3

## Initial Setup

### Cloning
Clone the repository _recursively_ to collect the dependencies as well as the source

```shell
git clone --recursive {your ssh/https link}
```

Note: If you already cloned the project the normal way
You may pull the dependencies by running

```shell
git submodule update --recursive
```

### Building
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

### Running
After building the application may launched from the project root directory with the following command:
```shell
cd ../
./build/visualization
```

## Resources

### Supported Model Formats
For the list of supported formats, see the [assimp wiki](https://github.com/assimp/assimp#supported-file-formats)

### Supported Texture Formats
Anything supported by [stb-image](https://github.com/nothings/stb/blob/master/stb_image.h).
The following is an excerpt from `stb_image.h` on support:

```
JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
PNG 1/2/4/8/16-bit-per-channel
TGA (not sure what subset, if a subset)
BMP non-1bpp, non-RLE
PSD (composited view only, no extra channels, 8/16 bit-per-channel)
GIF (*comp always reports as 4-channel)
HDR (radiance rgbE format)
PIC (Softimage PIC)
PNM (PPM and PGM binary only)
```

### Resource Paths
When the Visualizer is first launched it well search the directory the application is
in and the current working directory for the `resources/` directory.
If that fails, the Visualizer will prompt for the location of the `resources/` directory.

By default the Visualizer will search from the `resources/` directory the following:

Models in:
```shell
resources/models
```

Textures will be searched for recursively in the `resources/` directory, and _must have unique names_.
If two textures have the same name, it is undefined which one will be selected.

All searches for textures in models will follow the texture rules above.

## Controls

### Camera
The camera may be rotated by clicking and holding the left mouse button on the scene.

The camera may be moved by horizontally with `W`, `A`, `S`, & `D`,
 vertically with `Z`, & `X` and turned with the `Q`, & `E` keys

`W`: Forward

`A`: Left

`S`: Backward

`D`: Right

`Q`: Turn Left

`E`: Turn Right

`Z`: Up

`X`: Down

The camera controls may be changed from the 'Camera Settings' option in the 'Camera' menu

### Play/Pause
The simulation starts in a paused state, the scene will still render, but time will not advance until it is unpaused.

`P`: Pause/Resume scenario playback.
