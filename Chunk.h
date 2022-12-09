#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include <vector>

constexpr auto CHUNK_SIZE = 64;


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
	GLuint iID; // indices


private:
	char materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	static GLuint program;
	int getIndex(int x, int y, int z);
};