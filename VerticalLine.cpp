#include "VerticalLine.h"

GLuint VerticalLine::vao;
GLuint VerticalLine::vID;

void VerticalLine::render(GLuint modelViewID, float x, float z) {
	glm::mat4 modelView = glm::translate(glm::mat4(1), glm::vec3(x, 0, z));

	glUniformMatrix4fv(modelViewID, 1, GL_FALSE, glm::value_ptr(modelView));

	//glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vID); // bind the vertices
	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ARRAY_BUFFER, 0); // bind the vertices
	glBindVertexArray(0);
}

void VerticalLine::init() {
	GLfloat* vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * 6);

	vertices[0] = -0.1f;
	vertices[1] = 0.f;
	vertices[2] = 0.f;

	vertices[3] = -0.1;
	vertices[4] = 500.f;
	vertices[5] = 0.f;

	vertices[6] = 1.f;
	vertices[7] = 0.f;
	vertices[8] = 0.f;

	vertices[9] = 1.f;
	vertices[10] = 0.f;
	vertices[11] = 0.f;

	vertices[12] = -0.1;
	vertices[13] = 500.f;
	vertices[14] = 0.f;

	vertices[15] = -.1f;
	vertices[16] = 0.f;
	vertices[17] = 0.f;




	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vID); // bind vertices

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vID);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	free(vertices);
}

/// NEED DECONSTRUCTOR TO DEAL WITH FREEING UP vID and vao
/// can lead to some buffers not loading properly in the future, etc.
/// 