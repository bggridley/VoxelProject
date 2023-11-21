#pragma once


#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class VerticalLine {
public:
	static GLuint vID; // vbo
	static GLuint vao;

	static void init();
	static void render(GLuint modelViewID, float x, float z);

};