# Visualization
A 3D visualizer for ns-3 scenarios.

## Requirements

* A C++ 17 compliant compiler
  * Minimum supported compilers:
    * GCC 7.3.0
    * Clang 6.0.0
* CMake 3.12 or greater
* A graphics card supporting OpenGL 2
* The OpenSceneGraph plugin dependencies for [plugins](http://www.openscenegraph.org/index.php/documentation/guides/user-guides/61-osgplugins) you wish to use
  * See [the dependencies section](http://www.openscenegraph.org/index.php/download-section/dependencies) for more information
  * The FBX SDK is strongly recommended as it is very likely you will need to use FBX models. [Get it here](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-0)
    * See the FBX Models section for more information
  * The collada-dom library is necessary for reading COLLADA(.dae) files [Get it here](https://github.com/rdiankov/collada-dom)
    * See the DAE Models section for more information


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

### Setup OpenSceneGraph

#### FBX Models
If you're working with FBX models, download the FBX SDK for your platform [here](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-0)
and specify the path to the (installed) AutoDesk SDK in `FBX_DIR`

```shell
export FBX_DIR={where you installed the sdk}/AutoDeskSDK
```
Note, once OpenSceneGraph has been configured CMake will remember this location for as long as you keep the configured directory

#### DAE Models
COLLADA files are a common exchange format for 3D applications. For the application to be able to read DAE files
OpenSceneGraph will have to be build with [collada-dom](https://github.com/rdiankov/collada-dom) which will have to be built
from source.

Alternatively, if you wish to use a package instead there is a package available:
* Debian Based distros: `libcollada-dom2.4-dp-dev`
* Red Hat Based distros: `collada-dom-devel`

##### Building collada-dom
To build collada-dom you'll need CMake, Boost Filesystem, and Libxml2.

For more information see the [collada wiki](https://www.khronos.org/collada/wiki/DOM_guide:_Setting_up)
be careful though as some information seems dated.

To make linking easier when building later, we'll do an in-source build.
```shell
git clone https://github.com/rdiankov/collada-dom.git
cd collada-dom
cmake .
cmake --build .
```

##### Using collada-dom
When configuring OpenSceneGraph, specify the collada-dom source/build directory in `COLLADA_DIR`

```shell
export COLLADA_DIR=/path/to/collada-dom
```

If you installed a package (or installed after you built from source, then you can ignore this section)
Unfortunately, the project is set-up to be build only as a shared library,
so it'll have to be added to your `LD_LIBRARY_PATH` to load DAE models at runtime.
```shell
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/path/to/collada-dom
```


#### Building OpenSceneGraph
Move into `lib/OpenSceneGraph` and make a directory called `build` and cd into it
```shell
cd visualization/lib/OpenSceneGraph
mkdir build
cd build
```

Run `cmake` to configure and then build OpenSceneGraph.
Note: This will produce a `release` mode build
```shell
# configure
cmake ..
# build (add '-j' after the '.' for a parallel build)
cmake --build .
```

### Building the Visualizer
Move back to the project's root directory, and make a new directory called `build` and cd into it
```shell
# Move us up from lib/OpenSceneGraph/build
cd ../../..
mkdir build
cd build
```

Specify to CMake the location of OpenSceneGraph.
To do this the environment variables `OSG_ROOT` and `OSG_DIR` must be exported.

Set `OSG_ROOT` to the root directory of OpenSceneGraph e.g.:
```shell
export OSG_ROOT="/absolute/path/to/the/project/lib/OpenSceneGraph"
```


Set `OSG_DIR` to the build directory you created in the OpenSceneGraph directory.
You can set this from the `OSG_ROOT` from before e.g.:
```shell
export OSG_DIR="${OSG_ROOT}/build"
```

Run `cmake` and specify the source directory. Then run the build tool
```shell
cmake ..
cmake --build .
```

### Running the Visualizer

#### LD_LIBRARY_PATH
To run the visualizer, specify the built lib location of OpenSceneGraph and its plugins
in your `LD_LIBRARY_PATH`

The first path should be to `/absolute/path/to/the/project/lib/OpenSceneGraph/build/lib`
note the `build/lib` at the end of the path. This is the directory the *.so files were built

The second path `/absolute/path/to/the/project/lib/OpenSceneGraph/build/lib/osgPlugins-3.6.5`
This is the same as the previous path except the for the `osgPlugins-3.6.5` at the end.
Had you not included this path, the project would launch, but you'd be unable to load
any outside models.

Set your `LD_LIBRARY_PATH` based on the below. Note the `:` between paths.
If `LD_LIBRARY_PATH` is not currently set on your system, you may ignore the first portion
```shell
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/absolute/path/to/the/project/lib/OpenSceneGraph/build/lib:/absolute/path/to/the/project/lib/OpenSceneGraph/build/lib/osgPlugins-3.6.5"
```

Note: if you plan to load DAE files then you'll have to add collada-dom to your `LD_LIBRARY_PATH` as well,
unless you installed it of course.

See: [Using collada-dom](#using-collada-dom)

#### Model Paths
By default OpenSceneGraph will search from the current working directory for models.
To allow a root directory other than the current working directory
specify the `OSG_FILE_PATH` environment variable
```shell
OSG_FILE_PATH="/path/to/model/directory"
```

#### Execute the Command
Now with the `LD_LIBRARY_PATH` set to find OpenSceneGraph, the application may be launched.
```shell
./src/visualization
```