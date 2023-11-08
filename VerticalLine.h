#pragma once


#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"


class VerticalLine {
public:
	static GLuint vID; // vbo
	static GLuint vao;

	static void init();
	static void render(GLuint modelViewID, float x, float z);

};