#include "Chunk.h"
#include <iostream>
#include <GL/glew.h>
#include "SimplexNoise.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <vector>

GLuint Chunk::vID;
GLuint Chunk::program;
int Chunk::totalTriangles;

// call this statically
void Chunk::init(GLuint shader) {
	totalTriangles = 0;
	GLuint verticesCount = pow(CHUNK_SIZE, 3) * 8 * 3 * 3;
	GLuint* vertices = (GLuint*)malloc(sizeof(GLuint) * verticesCount);

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				int index = (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3 * 3;

				// FRONT FACE

				vertices[index] = x;
				vertices[index + 1] = y; 
				vertices[index + 2] = z;

				vertices[index + 3] = x;
				vertices[index + 4] = y; 
				vertices[index + 5] = z + 1;

				vertices[index + 6] = x;
				vertices[index + 7] = y + 1;
				vertices[index + 8] = z + 1;

				vertices[index + 9] = x;
				vertices[index + 10] = y + 1; 
				vertices[index + 11] = z;

				// BACK FACE

				vertices[index + 12] = x + 1; 
				vertices[index + 13] = y;
				vertices[index + 14] = z + 1;

				vertices[index + 15] = x + 1;
				vertices[index + 16] = y; 
				vertices[index + 17] = z;

				vertices[index + 18] = x + 1;
				vertices[index + 19] = y + 1; 
				vertices[index + 20] = z;

				vertices[index + 21] = x + 1;
				vertices[index + 22] = y + 1;
				vertices[index + 23] = z + 1;
			}
		}
	}

	std::cout << "Creating VBO one time." << std::endl;
	program = shader;
	// using just fill to avoid generating a new vertexID


	glGenBuffers(1, &vID); // bind vertices
	glBindBuffer(GL_ARRAY_BUFFER, vID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * verticesCount, vertices, GL_DYNAMIC_DRAW);

	std::cout << "vertices: " << verticesCount << " sizeof(vertices) :" << sizeof(vertices) << std::endl;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//std::cout << vID << std::endl;

	free(vertices);
}

Chunk::Chunk(int xo, int yo, int zo) {
	modelView = glm::translate(glm::mat4(1), glm::vec3(xo, yo, zo));

	//std::cout << pow(CHUNK_SIZE, 3) << std::endl;
	//indicesCount = pow(CHUNK_SIZE, 3) * 12 * 3;
	//GLuint* indices = (GLuint*)malloc(sizeof(GLuint) * indicesCount);

	//std::vector<GLuint> indices;

	int indicesIndex = 0;

	float scale = 300.0f;
	bool visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			float height = (SimplexNoise::noise((xo + x) / scale, (zo + z) / scale) + 1) / 2 * CHUNK_SIZE;
			for (int y = 0; y < CHUNK_SIZE; y++) {
				visited[x][y][z] = false;
				
				materials[x][y][z] = (y <= height) ? 1 : 0; //(rand() % 100) > 10 ? 1 : 0;
			}
		}
	}

	int x = 0, y = 0, z = 0;

	int sx, sy, sz;
	int ex, ey, ez;


	// naive greedy mesh..... epic dynamic progrraming

	if (true) {

		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					if (visited[x][y][z] || !materials[x][y][z]) continue;

					int endX = x;
					int endY = y;
					int endZ = z;

					visited[x][y][z] = true;

					while (endZ != CHUNK_SIZE - 1 && !visited[x][y][endZ + 1] && materials[x][y][endZ + 1]) {
						endZ++;
						visited[x][y][endZ] = true;
					}

					// try to expand on the y axis now.
					if (y + 1 != CHUNK_SIZE) {
						for (int j = y + 1; j < CHUNK_SIZE; j++) { // we know that it's fine at y. so now try y + 1 and so forh....
							bool fillY = true;

							for (int k = z; k <= endZ; k++) { // inclusive through endZ
								if (visited[x][j][k] || !materials[x][j][k]) {
									// so we hit a stop
									fillY = false;
									break;
								}
							}

							if (fillY) {
								endY = j;
								for (int k = z; k <= endZ; k++) {
									visited[x][j][k] = true;
								}
							}
							else {
								break;
							}
						}
					}

	
					if (x + 1 != CHUNK_SIZE) {
						for (int i = x + 1; i < CHUNK_SIZE; i++) {
							// so for every x we need to check the entire grid from (z to endZ) and (y to endY)
							bool done = false;


							for (int k = z; k <= endZ; k++) {
								if (done) break;

								for (int j = y; j <= endY; j++) {
									if (visited[i][j][k] || !materials[i][j][k]) {
										done = true;
										break;
									}
								}
							}

							if (done) {
								break;
							}
							else {
								endX = i;

								for (int k = z; k <= endZ; k++) {
									for (int j = y; j <= endY; j++) {
										visited[i][j][k] = true;
									}
								}
							}
						}
					}

					int bottomLeftFront = getIndex(x, y, z);
					int bottomRightFront = getIndex(x, y, endZ);

					int topLeftFront = getIndex(x, endY, z);
					int topRightFront = getIndex(x, endY, endZ);


					int bottomLeftBack = getIndex(endX, y, z);
					int bottomRightBack = getIndex(endX, y, endZ);

					int topLeftBack = getIndex(endX, endY, z);
					int topRightBack = getIndex(endX, endY, endZ);

					// front face

					indices.push_back((bottomLeftFront)+0);
					indices.push_back((bottomRightFront)+1);
					indices.push_back((topRightFront)+2);
					indices.push_back((topRightFront)+2);
					indices.push_back((topLeftFront)+3);
					indices.push_back((bottomLeftFront)+0);

					// back face

					indices.push_back((bottomRightBack)+4); // bottom left
					indices.push_back((bottomLeftBack)+5); // bottom right
					indices.push_back((topLeftBack)+6); // top right
					indices.push_back((topLeftBack)+6);
					indices.push_back((topRightBack)+7);
					indices.push_back((bottomRightBack)+4);

					// right face

					/*
					indices.push_back((bottomRightBack)+2);
					indices.push_back((bottomRightFront)+3);
					indices.push_back((topRightFront)+7);
					indices.push_back((topRightFront)+7); // right | x = 0 to 1, z 1 y 0 to 1
					indices.push_back((topRightBack)+6);
					indices.push_back((bottomRightBack)+2);

					// left face

					
					indices.push_back((bottomLeftFront)+0);
					indices.push_back((bottomLeftBack)+1);
					indices.push_back((topLeftBack)+5);
					indices.push_back((topLeftBack)+5);
					indices.push_back((topLeftFront)+4);
					indices.push_back((bottomLeftFront)+0);

					indices.push_back((bottomLeftFront)+0);
					indices.push_back((bottomRightFront)+3);
					indices.push_back((bottomRightBack)+2);
					indices.push_back((bottomRightBack)+2);
					indices.push_back((bottomLeftBack)+1);
					indices.push_back((bottomLeftFront)+0);

					indices.push_back((topLeftFront)+4);
					indices.push_back((topLeftBack)+5);
					indices.push_back((topRightBack)+6);
					indices.push_back((topRightBack)+6);
					indices.push_back((topRightFront)+7);
					indices.push_back((topLeftFront)+4);
					*/

				

				}
			}
		}

		totalTriangles += indices.size() / 3;
	}
	else {
	/*
		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				for (int y = 0; y < CHUNK_SIZE; y++) {
					int index = getIndex(x, y, z);


					bool draw = true;

					if (x > 0 && x < CHUNK_SIZE - 1 && y > 0 && y < CHUNK_SIZE - 1 && z > 0 && z < CHUNK_SIZE - 1) {
						if (materials[x + 1][y][z] && materials[x][y][z + 1] && materials[x - 1][y][z] && materials[x][y][z - 1] && materials[x][y + 1][z] && materials[x][y - 1][z]) {
							draw = false;
						}
					}

					if (materials[x][y][z] == 0) draw = false;


					if (draw) { // this statement reduces the amount of triangles by 10x.
						
						(index)+0;
						indices[indicesIndex++] = (index)+4;
						indices[indicesIndex++] = (index)+7; // front |  x = 0, z 0 to 1, y 0 to 1
						indices[indicesIndex++] = (index)+7;
						indices[indicesIndex++] = (index)+3;
						indices[indicesIndex++] = (index)+0;


						indices[indicesIndex++] = (index)+2;
						indices[indicesIndex++] = (index)+3;
						indices[indicesIndex++] = (index)+7;
						indices[indicesIndex++] = (index)+7; // right | x = 0 to 1, z 1 y 0 to 1
						indices[indicesIndex++] = (index)+6;
						indices[indicesIndex++] = (index)+2;


						indices[indicesIndex++] = (index)+1;
						indices[indicesIndex++] = (index)+2;
						indices[indicesIndex++] = (index)+6;
						indices[indicesIndex++] = (index)+6; //  back | x = 1, z 0 to 1, y 0 to 1
						indices[indicesIndex++] = (index)+5;
						indices[indicesIndex++] = (index)+1;

						indices[indicesIndex++] = (index)+0;
						indices[indicesIndex++] = (index)+1;
						indices[indicesIndex++] = (index)+5;
						indices[indicesIndex++] = (index)+5; // left
						indices[indicesIndex++] = (index)+4;
						indices[indicesIndex++] = (index)+0;

						indices[indicesIndex++] = (index)+0;
						indices[indicesIndex++] = (index)+3;
						indices[indicesIndex++] = (index)+2; // bottom
						indices[indicesIndex++] = (index)+2;
						indices[indicesIndex++] = (index)+1;
						indices[indicesIndex++] = (index)+0;

						indices[indicesIndex++] = (index)+4;
						indices[indicesIndex++] = (index)+5;
						indices[indicesIndex++] = (index)+6; // top
						indices[indicesIndex++] = (index)+6;
						indices[indicesIndex++] = (index)+7;
						indices[indicesIndex++] = (index)+4;
					}
				}
			}
		}
	*/
	}

	indicesCount = indices.size();
	totalTriangles += (indices.size() / 3); // each element represents a vertex. 3 vertices = triangle

	//glUseProgram(program);


	//free(indices);
}

int Chunk::getIndex(int x, int y, int z) {
	return (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3;
}



Chunk::~Chunk() {
	glDeleteBuffers(GL_ELEMENT_ARRAY_BUFFER, &iID);
	glDeleteBuffers(GL_ARRAY_BUFFER, &vID);
	glDeleteVertexArrays(1, &vao);
}