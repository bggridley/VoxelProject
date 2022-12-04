#include "Chunk.h"
#include <iostream>
#include <GL/glew.h>
#include "SimplexNoise.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

GLuint Chunk::vID;
GLuint Chunk::program;
int Chunk::totalTriangles;

// call this statically
void Chunk::init(GLuint shader) {
	totalTriangles = 0;
	GLuint verticesCount = pow(CHUNK_SIZE, 3) * 8 * 3;
	GLuint* vertices = (GLuint*)malloc(sizeof(GLuint) * verticesCount);

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				int index = (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3;
				vertices[index] = x;
				vertices[index + 1] = y; // 0 bottom left
				vertices[index + 2] = z;

				vertices[index + 3] = x + 1;
				vertices[index + 4] = y; // 1 bottom right
				vertices[index + 5] = z;

				vertices[index + 6] = x + 1;
				vertices[index + 7] = y; // 2 top right
				vertices[index + 8] = z + 1;

				vertices[index + 9] = x;
				vertices[index + 10] = y; // 3 top left
				vertices[index + 11] = z + 1;

				vertices[index + 12] = x; // 4
				vertices[index + 13] = y + 1;
				vertices[index + 14] = z;

				vertices[index + 15] = x + 1;
				vertices[index + 16] = y + 1; // 5
				vertices[index + 17] = z;

				vertices[index + 18] = x + 1;
				vertices[index + 19] = y + 1; // 6
				vertices[index + 20] = z + 1;

				vertices[index + 21] = x;
				vertices[index + 22] = y + 1; // 7
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
	this->xo = xo;
	this->yo = yo;
	this->zo = zo;

	modelView = glm::translate(glm::mat4(1), glm::vec3(xo, yo, zo));

	//std::cout << pow(CHUNK_SIZE, 3) << std::endl;
	indicesCount = pow(CHUNK_SIZE, 3) * 12 * 3;
	GLuint* indices = (GLuint*)malloc(sizeof(GLuint) * indicesCount);

	int indicesIndex = 0;

	bool visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				visited[x][y][z] = false;
				float noise = (SimplexNoise::noise((xo + x) / 100.f, (yo + y) / 100.f, (zo + z) / 100.f));
				materials[x][y][z] = (noise > 0) ? 1 : 0; //(rand() % 100) > 10 ? 1 : 0;
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

					indices[indicesIndex++] = (bottomLeftFront)+0;
					indices[indicesIndex++] = (topLeftFront)+4;
					indices[indicesIndex++] = (topRightFront)+7;
					indices[indicesIndex++] = (topRightFront)+7;
					indices[indicesIndex++] = (bottomRightFront)+3;
					indices[indicesIndex++] = (bottomLeftFront)+0;

					// back face

					indices[indicesIndex++] = (bottomLeftBack)+1; // bottom left
					indices[indicesIndex++] = (bottomRightBack)+2; // bottom right
					indices[indicesIndex++] = (topRightBack)+6; // top right
					indices[indicesIndex++] = (topRightBack)+6;
					indices[indicesIndex++] = (topLeftBack)+5;
					indices[indicesIndex++] = (bottomLeftBack)+1;

					// right face

					indices[indicesIndex++] = (bottomRightBack)+2;
					indices[indicesIndex++] = (bottomRightFront)+3;
					indices[indicesIndex++] = (topRightFront)+7;
					indices[indicesIndex++] = (topRightFront)+7; // right | x = 0 to 1, z 1 y 0 to 1
					indices[indicesIndex++] = (topRightBack)+6;
					indices[indicesIndex++] = (bottomRightBack)+2;

					// left face

					indices[indicesIndex++] = (bottomLeftFront)+0;
					indices[indicesIndex++] = (bottomLeftBack)+1;
					indices[indicesIndex++] = (topLeftBack)+5;
					indices[indicesIndex++] = (topLeftBack)+5;
					indices[indicesIndex++] = (topLeftFront)+4;
					indices[indicesIndex++] = (bottomLeftFront)+0;

					indices[indicesIndex++] = (bottomLeftFront)+0;
					indices[indicesIndex++] = (bottomRightFront)+3;
					indices[indicesIndex++] = (bottomRightBack)+2;
					indices[indicesIndex++] = (bottomRightBack)+2;
					indices[indicesIndex++] = (bottomLeftBack)+1;
					indices[indicesIndex++] = (bottomLeftFront)+0;

					indices[indicesIndex++] = (topLeftFront)+4;
					indices[indicesIndex++] = (topLeftBack)+5;
					indices[indicesIndex++] = (topRightBack)+6;
					indices[indicesIndex++] = (topRightBack)+6;
					indices[indicesIndex++] = (topRightFront)+7;
					indices[indicesIndex++] = (topLeftFront)+4;

				}
			}
		}
	}
	else {
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
						indices[indicesIndex++] = (index)+0;
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
	}

	totalTriangles += (indicesIndex / 3); // each element represents a vertex. 3 vertices = triangle


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	glGenBuffers(1, &iID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesIndex * sizeof(GL_UNSIGNED_INT), indices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vID); // bind the vertices

	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(GL_UNSIGNED_INT), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	free(indices);
}

int Chunk::getIndex(int x, int y, int z) {
	return (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8;
}


void Chunk::render(GLuint modelViewID) {
	//glUseProgram(program);

	glUniformMatrix4fv(modelViewID, 1, GL_FALSE, glm::value_ptr(modelView));






	//glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);

	//glTranslatef(xo, yo, zo);
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (void*)0);

	//glDrawArrays(GL_TRIANGLES, 0, indicesCount);

	//glDrawArrays(GL_TRIANGLES, 0, indicesCount);

	glBindVertexArray(0);

	//glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}


Chunk::~Chunk() {
}