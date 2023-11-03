#include "Chunk.h"
#include "World.h"
#include <iostream>
#include <GL/glew.h>
#include "SimplexNoise.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <chrono>
#include <vector>


GLuint Chunk::vID;
GLuint Chunk::tID;
GLuint Chunk::program;
GLuint Chunk::verticesCount;
GLuint Chunk::texCount;
int Chunk::totalTriangles;
int Chunk::totalCount;
std::chrono::duration<double> Chunk::totalTime;

// call this statically
void Chunk::init(GLuint shader) {
	totalTime = std::chrono::duration<double>::zero();
	totalTriangles = 0;
	verticesCount = pow(CHUNK_SIZE, 3) * 8 * 3 * 3;
	texCount = pow(CHUNK_SIZE, 3) * 8 * 3 * 2;

	GLuint* vertices = (GLuint*)malloc(sizeof(GLuint) * verticesCount);
	GLfloat* texCoords = (GLfloat*)malloc(sizeof(GLfloat) * texCount);
	//if (!vertices) return;

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				int index = (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3 * 3;
				int tex = (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3 * 2;

				// FRONT FACE

				texCoords[tex++] = z;
				texCoords[tex++] = y;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = y;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = y + 1;

				texCoords[tex++] = z;
				texCoords[tex++] = y + 1;



				texCoords[tex++] = z + 1;
				texCoords[tex++] = y;

				texCoords[tex++] = z;
				texCoords[tex++] = y;

				texCoords[tex++] = z;
				texCoords[tex++] = y + 1;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = y + 1;

				// RIGHT

				texCoords[tex++] = x;
				texCoords[tex++] = y;

				texCoords[tex++] = x + 1;
				texCoords[tex++] = y;

				texCoords[tex++] = x + 1;
				texCoords[tex++] = y + 1;

				texCoords[tex++] = x;
				texCoords[tex++] = y + 1;

				// LEFT
				texCoords[tex++] = x + 1;
				texCoords[tex++] = y;

				texCoords[tex++] = x;
				texCoords[tex++] = y;

				texCoords[tex++] = x;
				texCoords[tex++] = y + 1;

				texCoords[tex++] = x + 1;
				texCoords[tex++] = y + 1;

				//TOP
				texCoords[tex++] = z;
				texCoords[tex++] = x;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = x;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = x + 1;

				texCoords[tex++] = z;
				texCoords[tex++] = x + 1;

				//BOTTOM
				texCoords[tex++] = z;
				texCoords[tex++] = x + 1;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = x + 1;

				texCoords[tex++] = z + 1;
				texCoords[tex++] = x;

				texCoords[tex++] = z;
				texCoords[tex++] = x;

				vertices[index] = x;
				vertices[index + 1] = y;  // 0
				vertices[index + 2] = z;

				vertices[index + 3] = x;
				vertices[index + 4] = y; // 1
				vertices[index + 5] = z + 1;

				vertices[index + 6] = x;
				vertices[index + 7] = y + 1; // 2
				vertices[index + 8] = z + 1;

				vertices[index + 9] = x;
				vertices[index + 10] = y + 1;  // 3
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

				// RIGHT FACE

				vertices[index + 24] = x;
				vertices[index + 25] = y;
				vertices[index + 26] = z + 1;

				vertices[index + 27] = x + 1;
				vertices[index + 28] = y;
				vertices[index + 29] = z + 1;

				vertices[index + 30] = x + 1;
				vertices[index + 31] = y + 1;
				vertices[index + 32] = z + 1;

				vertices[index + 33] = x;
				vertices[index + 34] = y + 1;
				vertices[index + 35] = z + 1;

				//LEFT FACE

				vertices[index + 36] = x + 1;
				vertices[index + 37] = y;
				vertices[index + 38] = z;

				vertices[index + 39] = x;
				vertices[index + 40] = y;
				vertices[index + 41] = z;

				vertices[index + 42] = x;
				vertices[index + 43] = y + 1;
				vertices[index + 44] = z;

				vertices[index + 45] = x + 1;
				vertices[index + 46] = y + 1;
				vertices[index + 47] = z;

				// TOP FACE

				vertices[index + 48] = x;
				vertices[index + 49] = y + 1;
				vertices[index + 50] = z;

				vertices[index + 51] = x;
				vertices[index + 52] = y + 1;
				vertices[index + 53] = z + 1;

				vertices[index + 54] = x + 1;
				vertices[index + 55] = y + 1;
				vertices[index + 56] = z + 1;

				vertices[index + 57] = x + 1;
				vertices[index + 58] = y + 1;
				vertices[index + 59] = z;

				// BOTTOM  FACE
				vertices[index + 60] = x + 1;
				vertices[index + 61] = y;
				vertices[index + 62] = z;

				vertices[index + 63] = x + 1;
				vertices[index + 64] = y;
				vertices[index + 65] = z + 1;

				vertices[index + 66] = x;
				vertices[index + 67] = y;
				vertices[index + 68] = z + 1;

				vertices[index + 69] = x;
				vertices[index + 70] = y;
				vertices[index + 71] = z;

				//std::cout << index << std::endl;

			}
		}
	}



	std::cout << "Creating VBO one time." << std::endl;
	program = shader;
	// using just fill to avoid generating a new vertexID






	glGenBuffers(1, &vID); // bind vertices
	glBindBuffer(GL_ARRAY_BUFFER, vID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * verticesCount, vertices, GL_DYNAMIC_DRAW);


	glGenBuffers(1, &tID);
	glBindBuffer(GL_ARRAY_BUFFER, tID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCount, texCoords, GL_DYNAMIC_DRAW);


	std::cout << "vertices: " << verticesCount << " sizeof(vertices) :" << sizeof(vertices) << std::endl;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//std::cout << vID << std::endl;

	free(vertices);
	free(texCoords);
}

Chunk::Chunk(int xo, int yo, int zo) {
	this->xo = xo;
	this->yo = yo;
	this->zo = zo;

	modelView = glm::translate(glm::mat4(1), glm::vec3(xo, yo, zo));
	char materials[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	//std::cout << pow(CHUNK_SIZE, 3) << std::endl;
	//indicesCount = pow(CHUNK_SIZE, 3) * 12 * 3;
	//std::vector<GLuint> indices;

	//std::vector<GLuint> indices;

	auto start = std::chrono::high_resolution_clock::now();

	bool visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	//bool occlusion[CHUNK_SIZE][CHUNK_SIZE];

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {

			auto noise1 = std::chrono::high_resolution_clock::now();
			float height = World::getHeight(xo + x, zo + z);

			auto noise2 = std::chrono::high_resolution_clock::now();

			//std::chrono::duration<double> ddd = noise2 - noise1;
			//std::cout << ddd.count() << std::endl;
			//float height2 = ((SimplexNoise::noise((xo + x) / scale, (zo + z) / scale) + 1) / 2 * CHUNK_SIZE);

			//std::cout << height << ", " << height2 << std::endl;// +((SimplexNoise::noise((xo + x) / 100, (zo + z) / 100) + 1) / 2 * CHUNK_SIZE);
			for (int y = 0; y < CHUNK_SIZE; y++) {
				//int index = (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3 * 2;

				visited[x][y][z] = false;

				materials[x][y][z] = (y + yo <= height) ? 1 : 0; //(rand() % 100) > 10 ? 1 : 0;

				// need to switch to an octree

				if (materials[x][y][z] == 1) empty = false;
			}
		}
	}

	auto end1 = std::chrono::high_resolution_clock::now();

	//std::chrono::duration<double> duration = end1 - start;
	//std::cout << duration.count() << std::endl;

	int x = 0, y = 0, z = 0;

	// naive greedy mesh..... epic dynamic progrraming
	// LOD - decreasae level of detail
	//performaance measuring here::



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

				z = endZ;

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
				indices.push_back((bottomRightFront)+8); // bottom left
				indices.push_back((bottomRightBack)+9); // bottom right
				indices.push_back((topRightBack)+10); // top right
				indices.push_back((topRightBack)+10);
				indices.push_back((topRightFront)+11);
				indices.push_back((bottomRightFront)+8);


				// LEFT

				indices.push_back((bottomLeftBack)+12); // bottom left
				indices.push_back((bottomLeftFront)+13); // bottom right
				indices.push_back((topLeftFront)+14); // top right
				indices.push_back((topLeftFront)+14);
				indices.push_back((topLeftBack)+15);
				indices.push_back((bottomLeftBack)+12);


				indices.push_back((topLeftFront)+16);
				indices.push_back((topRightFront)+17);
				indices.push_back((topRightBack)+18);
				indices.push_back((topRightBack)+18);
				indices.push_back((topLeftBack)+19);
				indices.push_back((topLeftFront)+16);

				indices.push_back((bottomLeftBack)+20);
				indices.push_back((bottomRightBack)+21);
				indices.push_back((bottomRightFront)+22);
				indices.push_back((bottomRightFront)+22);
				indices.push_back((bottomLeftFront)+23);
				indices.push_back((bottomLeftBack)+20);
			}
		}
	}

	totalTriangles += indices.size() / 3;

	indicesCount = indices.size();
	totalTriangles += (indices.size() / 3); // each element represents a vertex. 3 vertices = triangle

	auto end2 = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> duration = end2 - end1;
	totalCount++;
	totalTime += duration;
	//std::cout << duration.count() << std::endl;
	//glUseProgram(program);


	//free(indices);
}

int Chunk::getIndex(int x, int y, int z) {
	return (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8 * 3;
}



Chunk::~Chunk() {
	//glDeleteBuffers(GL_ELEMENT_ARRAY_BUFFER, &iID);
	//if(vID != 0)
	//glDeleteBuffers(GL_ARRAY_BUFFER, &vID);

	//if (&tID != 0)
	//glDeleteBuffers(GL_ARRAY_BUFFER, &tID); 


	glBindVertexArray(0);




	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &iID);

}