#include "Mesh.h"
#include <stdlib.h>

void Mesh::init(GLfloat* vertices, GLuint verticesCount, GLfloat* colors, GLuint colorsCount, GLuint* indices, GLuint indicesCount) {
	this->indicesCount = indicesCount;
	//this->indices = indices;

	fillVertices(vertices, verticesCount);
	

	if (iID == 0) {
		glGenBuffers(1, &iID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(GLuint), indices, GL_DYNAMIC_DRAW);
	}

	if (cID == 0) {
		glGenBuffers(1, &cID);

		glBindBuffer(GL_ARRAY_BUFFER, cID);
		glBufferData(GL_ARRAY_BUFFER, colorsCount * sizeof(GLfloat), colors, GL_DYNAMIC_DRAW);
	}

	/*
	float vertices[12] = {
		-100, 0, 100,
		-100, 0, -100,
		100, 0, 100,
		100, 0, -100};

	float colors[12] = { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 };
	indicesCount = 6;
	float* indices[6] = { 0, 1, 3, 3, 2, 0 }
	*/

	// if these arrays become passed as parameters ( when I add an entity or something ), then
	// then an additional size parameter needs to be passe

	unbind();
	//free(vertices); FREE THIS LATER FRO SURE
	free(colors);
}

Mesh::Mesh() {
	
}

void Mesh::fillVertices(GLfloat* vertices, GLuint verticesCount) {
	fill(vID, GL_ARRAY_BUFFER, vertices, verticesCount);
}

void Mesh::fillIndices(GLuint* indices, GLuint indicesCount) {
	this->indicesCount = indicesCount;
	fill(iID, GL_ELEMENT_ARRAY_BUFFER, indices, indicesCount);
}

template<typename T>
void Mesh::fill(GLuint& id, int type, T* data, GLuint count) {
	if (id == 0) {
		glGenBuffers(1, &id);
	}

	glBindBuffer(type, id);
	glBufferData(type, count * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);
}

void Mesh::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // unbind 
}

void Mesh::render(GLfloat xo, GLfloat yo, GLfloat zo) {
	glPushMatrix();
	
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glTranslatef(xo, yo, zo);

	//glCullFace(GL_BACK);

	
	//glBindTexture(GL_TEXTURE_2D, atlas.getTexture().getTexID());
	//glTexCoordPointer(2, GL_FLOAT, 0, 0);arlClear

	    
	//glEnableVertexAttribArray(0); // attribute 1 aka position

	glBindBuffer(GL_ARRAY_BUFFER, vID);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cID);
	glColorPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iID);

	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (void*) 0);
	//glDisableVertexAttribArray(0); // attribute
	glBindVertexArray(0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	//glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glPopMatrix();
}

Mesh::~Mesh() {
	if (vID != 0) {
		glDeleteBuffers(1, &vID); // delete buffer
	}

	if (iID != 0) {
		glDeleteBuffers(1, &iID); // delete buffer; if tihis is not done, weird things can happen
	}

	if (cID != 0) {
		glDeleteBuffers(1, &cID); // delete buffer; if tihis is not done, weird things can happen
	}

	//free(indices);
}