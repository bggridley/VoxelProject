#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"

constexpr auto CHUNK_SIZE = 32;

class Chunk {
public:
	Chunk(int xOffset, int yOffset, int zOffset);
	~Chunk();
	void render(GLuint modelViewID);
	static void init(GLuint shader);
private:
	glm::mat4 modelView;
	char materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	int xo, yo, zo;
	void generateVertices();
	void generateIndices();
	GLuint indicesCount;
	static GLuint vID; // vertices
	GLuint iID; // indices
	GLuint vao;
	static GLuint program;
	int getIndex(int x, int y, int z);
};