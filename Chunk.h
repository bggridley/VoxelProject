#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"

constexpr auto CHUNK_SIZE = 32;


class Chunk {
public:
	GLuint indicesCount;
	Chunk(int xOffset, int yOffset, int zOffset);
	~Chunk();
	void render(GLuint modelViewID);
	static void init(GLuint shader);
	static int totalTriangles;
	GLuint vao;
	glm::mat4 modelView;
private:
	char materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	int xo, yo, zo;
	void generateVertices();
	void generateIndices();

	static GLuint vID; // vertices
	GLuint iID; // indices
	
	static GLuint program;
	int getIndex(int x, int y, int z);
};