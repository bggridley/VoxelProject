#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include <vector>
#include <chrono>

constexpr auto CHUNK_SIZE = 72;


class Chunk {
public:
	GLuint indicesCount;
	Chunk(int xOffset, int yOffset, int zOffset);
	~Chunk();
	static void init(GLuint shader);
	static int totalTriangles;
	GLuint vao;
	glm::mat4 modelView;
	std::vector<GLuint> indices;
	static GLuint vID; // vertices

	static int totalCount;
	static std::chrono::duration<double> totalTime; 
	GLuint iID; // indices
	static GLuint tID; // figure out a better way to do this later...
	static GLuint verticesCount;
	static GLuint texCount;
	bool empty = true;
	int xo, yo, zo;
	//char materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

private:
	
	static GLuint program;
	int getIndex(int x, int y, int z);
};