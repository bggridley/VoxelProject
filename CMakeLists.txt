cmake_minimum_required(VERSION 3.25)
project(VoxelProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)



add_executable(VoxelProject
        BackgroundChunk.h
        Chunk.cpp
        Chunk.h
        DAG.h
        Edge.h
        PoissonSampler.cpp
        PoissonSampler.h
        Primitives.h
        SimplexNoise.cpp
        SimplexNoise.h
        stb_image.h
        Triangle.h
        VerticalLine.cpp
        VerticalLine.h
        Voxels.cpp
        World.cpp
        World.h)

#https://stackoverflow.com/a/46995969
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/texture.png
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(VoxelProject PUBLIC ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
target_link_libraries(VoxelProject ${OPENGL_LIBRARY} ${GLEW_LIBRARIES} glfw)