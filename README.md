# VoxelProject
	Fun 3D voxel world using glfw, glew & glm!

[![VoxelProject Video](https://img.youtube.com/vi/5QcHtZsGygo/1.jpg)](https://www.youtube.com/watch?v=5QcHtZsGygo)
[![VoxelProject Video 2](https://img.youtube.com/vi/i-36rOuVSHw/3.jpg)](https://www.youtube.com/watch?v=i-36rOuVSHw)

^ See videos (YT compression made quality bad).

## Features
 - LOD System - *Comp Geo final project ft. [Stephen Derenski](https://github.com/sderenski)*
	- Randomized Incremental Delaunay Triangulation
	- Bowyer-Watson method (see BackgroundChunk.h)
	- Poisson Sampling for spaced random distributions (PoissonSampling.cpp)
	- **TODO** stitch the Delaunay mesh together with the existing voxel terrain for seamless gameplay
- Frustum Culling
- Dynamic naive chunk meshing (Chunk.cpp)

## Delaunay time analysis

![Time analysis](https://i.imgur.com/MvURnlA.png)

Looks like *O(nlogn)* :)\
If you would like to build and run the program, I would recommend using Visual Studio. No makefile included.

## Running on Linux

Same dependencies as below are required.

In the base folder, run:

```shell
cmake .

cmake --build .
```

This will build an executable. To run it:

```shell
./VoxelProject
```

### Dependencies
- Stb-master (included)
- [GLEW 2.1.10](https://glew.sourceforge.net/)
- [GLFW 3.3.8](https://www.glfw.org/version-3.3.8-released.html)
- [glm 4.20.8](https://glmatrix.net/)

### Linker, misc.
- You will need to define various linker settings to get the libraries working properly
- See .vxproj files
- Also required is a 16x16 texture located at C:\texture.png
	- If you put this file somewhere else, you must change the path within Voxels.cpp 
