#pragma once
#include <GL/glew.h>

class Mesh {

public:
	Mesh();
	void init(GLfloat* vertices, GLuint verticesCount, GLfloat* colors, GLuint colorsCount, GLuint* indices, GLuint indicesCount);
	~Mesh();
	void render(GLfloat xo, GLfloat yo, GLfloat zo);
	//GLuint* indices;

	template <typename T>
	static void fill(GLuint& id, int type, T* data, GLuint count);
	void fillVertices(GLfloat* vertices, GLuint verticesCount);
	void fillIndices(GLuint* indices, GLuint indicesCount);
	static void unbind();
	GLuint vID = 0;
	GLuint iID = 0;
	GLuint cID = 0;
private:
	int indicesCount;






};
