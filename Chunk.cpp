#include "Chunk.h"
#include <iostream>
#include <GL/glew.h>
#include "SimplexNoise.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

GLuint Chunk::vID;
GLuint Chunk::program; 

// call this statically
void Chunk::init(GLuint shader) {
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

	std::cout << "once " << std::endl;
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
	GLuint colorsCount = pow(CHUNK_SIZE, 3) * 8 * 3;


	//GLfloat* vertices = (GLfloat*) malloc(sizeof(GLfloat) * verticesCount); // 4 vertices per cube
	GLfloat* colors = (GLfloat*)malloc(sizeof(GLfloat) * colorsCount);
	GLuint* indices = (GLuint*)malloc(sizeof(GLuint) * indicesCount);

	int indicesIndex = 0;
	//indices[indicesIndex++] = 0;
	//indices[indicesIndex++] = 92160 / 3;
	//indices[indicesIndex++] = 92520 / 3;

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				float noise = (SimplexNoise::noise((xo + x) / 100.f, (yo + y) / 100.f, (zo + z) / 100.f));
				materials[x][y][z] = (noise > 0) ? 1 : 0; //(rand() % 100) > 10 ? 1 : 0;
			}
		}
	}

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < CHUNK_SIZE; y++) {
				int index = getIndex(x, y, z) * 3;

				//if (x == 0 && z == CHUNK_SIZE - 1 && y == 0) std::cout << index << std::endl;
				//if (x == CHUNK_SIZE - 1 && z == CHUNK_SIZE - 1 && y == 0) std::cout << index << std::endl;
				float c = (rand() % 100) / 100.f;

				float r = (rand() % 100) / 100.f;
				float g = (rand() % 100) / 100.f;
				float b = (rand() % 100) / 100.f;

				for (int i = 0; i < 8; i++) {
					colors[index + (i * 3)] = r;
					colors[index + (i * 3) + 1] = g;
					colors[index + (i * 3) + 2] = b;
				}


				//	if (c > 0.9) {
				bool draw = true;

				if (x > 0 && x < CHUNK_SIZE - 1 && y > 0 && y < CHUNK_SIZE - 1 && z > 0 && z < CHUNK_SIZE - 1) {
					if (materials[x + 1][y][z] && materials[x][y][z + 1] && materials[x - 1][y][z] && materials[x][y][z - 1] && materials[x][y + 1][z] && materials[x][y - 1][z]) {
						draw = false;
					}
				}

				if (materials[x][y][z] == 0) draw = false;


				if (draw) {
					indices[indicesIndex++] = (index / 3) + 0;
					indices[indicesIndex++] = (index / 3) + 4;
					indices[indicesIndex++] = (index / 3) + 7; // left
					indices[indicesIndex++] = (index / 3) + 7;
					indices[indicesIndex++] = (index / 3) + 3;
					indices[indicesIndex++] = (index / 3) + 0;


					indices[indicesIndex++] = (index / 3) + 2;
					indices[indicesIndex++] = (index / 3) + 3;
					indices[indicesIndex++] = (index / 3) + 7;
					indices[indicesIndex++] = (index / 3) + 7; // back
					indices[indicesIndex++] = (index / 3) + 6;
					indices[indicesIndex++] = (index / 3) + 2;


					indices[indicesIndex++] = (index / 3) + 1;
					indices[indicesIndex++] = (index / 3) + 2;
					indices[indicesIndex++] = (index / 3) + 6;
					indices[indicesIndex++] = (index / 3) + 6; //  right
					indices[indicesIndex++] = (index / 3) + 5;
					indices[indicesIndex++] = (index / 3) + 1;

					indices[indicesIndex++] = (index / 3) + 0;
					indices[indicesIndex++] = (index / 3) + 1;
					indices[indicesIndex++] = (index / 3) + 5;
					indices[indicesIndex++] = (index / 3) + 5; // front
					indices[indicesIndex++] = (index / 3) + 4;
					indices[indicesIndex++] = (index / 3) + 0;



					
					indices[indicesIndex++] = (index / 3) + 0;
					indices[indicesIndex++] = (index / 3) + 3;
					indices[indicesIndex++] = (index / 3) + 2; // bottom
					indices[indicesIndex++] = (index / 3) + 2;
					indices[indicesIndex++] = (index / 3) + 1;
					indices[indicesIndex++] = (index / 3) + 0;
					
					indices[indicesIndex++] = (index / 3) + 4;
					indices[indicesIndex++] = (index / 3) + 5;
					indices[indicesIndex++] = (index / 3) + 6; // top
					indices[indicesIndex++] = (index / 3) + 6;
					indices[indicesIndex++] = (index / 3) + 7;
					indices[indicesIndex++] = (index / 3) + 4;
				}
			}
		}
	}


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

	//chunkMesh = new 
	// (program, vao);
	//chunkMesh->vID = vID;

	//chunkMesh->fill(chunkMesh->cID, GL_ARRAY_BUFFER, colors, colorsCount);
	//chunkMesh->fillIndices(indices, indicesIndex);
	//Mesh::unbind(); // use after fills

	free(colors);
	free(indices);
	//chunkMesh::init();
}

int Chunk::getIndex(int x, int y, int z) {
	return (x + CHUNK_SIZE * (y + CHUNK_SIZE * z)) * 8;
}


void Chunk::render(GLuint modelViewID) {
	glUseProgram(program);

	glUniformMatrix4fv(modelViewID, 1, GL_FALSE, glm::value_ptr(modelView));
	

	



	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);

	//glTranslatef(xo, yo, zo);
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (void*)0);

	//glDrawArrays(GL_TRIANGLES, 0, indicesCount);

	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}


Chunk::~Chunk() {
}