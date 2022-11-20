#pragma once
#include "Mesh.h"

constexpr auto CHUNK_SIZE = 16;

class Chunk {
public:
	Chunk(int xOffset, int yOffset, int zOffset);
	Mesh* chunkMesh;
	~Chunk();
	void render();
	static void init();
private:
	int materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	int xo, yo, zo;
	void generateVertices();
	void generateIndices();
	static GLuint vID;
	int getIndex(int x, int y, int z);
};