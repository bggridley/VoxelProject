#include <iostream>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Chunk.h"
#include <stdlib.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

bool left = false, right = false, forward = false, backward = false, space = false, shift = false;
float xrotation = 0;
float yrotation = 0;
bool vsync = true;
double mouseX, mouseY;
int totalUpdates = 0;
bool wireframe = false;
const int s = 4; // dimension of chunk grid

// Shaders
const GLchar* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"uniform mat4 projectionMatrix;\n"
"uniform mat4 viewMatrix;\n"
"uniform mat4 modelViewMatrix;\n"
//"layout (location = 1) in vec3 color;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"gl_Position = projectionMatrix * viewMatrix * modelViewMatrix * vec4(position, 1.0);\n"
"ourColor = position / 120.0;\n"
"}\0";
const GLchar* fragmentShaderSource =
"#version 330 core\n"
"in vec3 ourColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(ourColor, 1.0);\n"
"}\n\0";

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_RELEASE) {
		bool val = (action == GLFW_PRESS);
		switch (key) {
		case GLFW_KEY_A:
			left = val;
			break;
		case GLFW_KEY_D:
			right = val;
			break;
		case GLFW_KEY_W:
			forward = val;
			break;
		case GLFW_KEY_S:
			backward = val;
			break;
		case GLFW_KEY_SPACE:
			space = val;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shift = val;
			break;
		case GLFW_KEY_V:
			if (val) {
				vsync = !vsync;
				glfwSwapInterval(vsync);
			}
			break;
		case GLFW_KEY_K:
			if (val) {
				wireframe = !wireframe;

				if (wireframe)
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			break;
		}
	}
}

int main(int argc, char** argv) {


	Game* game = new Game(); // create a new game instance
	//std::cout << mode.w << " , " << mode.h << std::endl;

	if (GLFW_TRUE != glfwInit()) {
		std::cout << "Failed to initialize GLFW." << std::endl;
	}
	else {
		std::cout << "GLFW initialized succssfully." << std::endl;
	}

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);



	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);




	game->setDimensions(mode->width / 1.5, mode->height / 1.5);
	game->setFullscreenDimensions(mode->width, mode->height);
	game->window = glfwCreateWindow(game->getWidth(), game->getHeight(), "Voxels", NULL, NULL);

	glfwSetWindowPos(game->window, (mode->width - mode->width / 1.5) / 2, (mode->height - mode->height / 1.5) / 2);
	glfwMakeContextCurrent(game->window);
	glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	glfwSetKeyCallback(game->window, keyboard);
	//glfwSetCursorPosCallback(game->window, mouse);

	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to intitialize GLEW" << std::endl;
	}
	else {
		std::cout << "GLEW intiailized successfully" << std::endl;
	}







	glm::vec3 fw = { 1.0, 0.0, 0.0 };
	glm::vec3 eye = { 0.0, 0.04, 0.0 };
	glm::vec3 up = { 0.0, 1.0, 0.0 };

	glViewport(0, 0, game->getWidth(), game->getHeight());


	/*

	Compile shaders

	*/

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Check for compile time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Failed to compile vertex shader.\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Check for compile time errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "Failed to compile fragment shader.\n" << infoLog << std::endl;
	}
	// Link shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "Failed to create program.\n" << infoLog << std::endl;
	}
	else {
		std::cout << "Successfully created shader program: " << shaderProgram << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, game->getWidth() / game->getHeight(), 0.1, 400.0);
	*/

	glm::mat4 projection = glm::perspective(glm::radians<float>(45.0f), game->getWidth() / game->getHeight(), 0.1f, 400.0f);

	int curFPS = 0;
	int updates = 0;
	int frames = 0;
	const int fps = 60;
	float t = 0.0;

	float dt = 1.0f / 60.0f; // target ups regardless of fps
	float deltaBuffer = 0.0f;
	float deltaTime = 0.0f;
	float currentTime = glfwGetTime(); // gotta do this so it doesnt blow up
	float accumulator = 0.0f;

	float rot = 0.0f;

	// the amount of chunks on each axis


	// pass this by value because it makes no sense to pass by reference if shaderprogram is on the stack....
	Chunk::init(shaderProgram); // one single time

	Chunk* chunks[s][s][s];

	GLuint cID = 0;
	glGenBuffers(1, &cID);
	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				chunks[x][y][z] = new Chunk(x * CHUNK_SIZE, y * CHUNK_SIZE, z * CHUNK_SIZE);
			}
		}
	}

	std::cout << "Total Triangles: " << Chunk::totalTriangles << std::endl;

	glUseProgram(shaderProgram);
	GLuint projectionID = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));
	GLuint viewID = glGetUniformLocation(shaderProgram, "viewMatrix");
	GLuint modelViewID = glGetUniformLocation(shaderProgram, "modelViewMatrix");

	while (!glfwWindowShouldClose(game->window))
	{
		if (t >= 1.0f) {
			t -= 1.0f;
			std::cout << "FPS: " << frames << " | UPS: " << updates << std::endl;
			curFPS = frames;
			frames = 0;
			updates = 0;
		}

		float newTime = glfwGetTime();
		deltaTime = float(newTime - currentTime);
		currentTime = newTime;
		accumulator += deltaTime;

		while (accumulator >= dt) {
			updates++;
			totalUpdates++;
			glfwPollEvents();

			double mx, my;
			glfwGetCursorPos(game->window, &mx, &my);

			if (totalUpdates > 60) {
				yrotation += (mouseX - mx) * 0.001;
				xrotation += (mouseY - my) * 0.001;
			}

			mouseX = mx;
			mouseY = my;


			if (xrotation > 3.14 / 2) xrotation = 3.14 / 2;
			if (xrotation < -3.14 / 2) xrotation = -3.14 / 2;

			fw.x = cos(yrotation) * cos(xrotation);
			fw.y = sin(xrotation);
			fw.z = cos(xrotation) * sin(-yrotation);

			//std::cout << fw.x << ", " << fw.y << ", " << fw.z << std::endl;

			if (forward) {
				eye += fw;
			}

			if (backward) {
				eye -= fw;
			}

			if (left) {
				eye -= normalize(cross(fw, up));//.normalize();
			}

			if (right) {
				eye += normalize(cross(fw, up));
			}

			if (space) {
				eye += up;
			}

			if (shift) {
				eye -= up;
			}

			t += dt;
			accumulator -= dt;
			rot += dt;
		}

		float alpha = accumulator / dt;

		glClearColor(0.5f, 0.5f, 1.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		//glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();
		//gluLookAt(eye[0], eye[1], eye[2], eye[0] + fw[0], eye[1] + fw[1], eye[2] + fw[2], up[0], up[1], up[2]);




		glm::mat4 view = glm::lookAt(
			eye, // Camera is at (4,3,3), in World Space
			(eye + fw), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);


		//glm::inverse(view, view);

		glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CCW);

		






		//glEnable(GL_BLEND);
		

		//glDisable(GL_BLEND);
	


		for (int x = 0; x < s; x++) {
			for (int y = 0; y < s; y++) {
				for (int z = 0; z < s; z++) {
					glUniformMatrix4fv(modelViewID, 1, GL_FALSE, glm::value_ptr(chunks[x][y][z]->modelView));

					//glTranslatef(xo, yo, zo);
					glBindVertexArray(chunks[x][y][z]->vao);

					glDrawElements(GL_TRIANGLES, chunks[x][y][z]->indicesCount, GL_UNSIGNED_INT, (void*)0);

					glBindVertexArray(0);
					//chunks[x][y][z]->render(modelViewID);
				}
			}
		}

		glDisable(GL_CULL_FACE);

		frames++;

		glfwSwapBuffers(game->window);
	}

	glfwTerminate();
	return 0;
}