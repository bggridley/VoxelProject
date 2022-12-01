#include <iostream>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Mesh.h"
#include "Chunk.h"
#include <stdlib.h>
#include "Vector3.h"

bool left = false, right = false, forward = false, backward = false, space = false, shift = false;
float xrotation = 0;
float yrotation = 0;
bool vsync = true;
double mouseX, mouseY;
int totalUpdates = 0;

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
		}
	}
}

int main(int argc, char** argv) {


	Game* game = new Game(); // create a new game instance
	//std::cout << mode.w << " , " << mode.h << std::endl;

	glfwInit();


	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	std::cout << monitor << std::endl;
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);


	game->setDimensions(mode->width / 1.5, mode->height / 1.5);
	game->setFullscreenDimensions(mode->width, mode->height);
	game->window = glfwCreateWindow(game->getWidth(), game->getHeight(), "Voxels", NULL, NULL);
	glfwMakeContextCurrent(game->window);
	glfwSetInputMode(game->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetKeyCallback(game->window, keyboard);
	//glfwSetCursorPosCallback(game->window, mouse);

	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to intitialize GLEW" << std::endl;
	}
	else {
		std::cout << "GLEW intiailized successfully" << std::endl;
	}

	glewExperimental = GL_TRUE;

	Vector3 fw = {1.0, 0.0, 0.0};
	Vector3 eye = Vector3(0.0, 0.04, 0.0);
	Vector3 up = {0.0, 1.0, 0.0};

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, game->getWidth() / game->getHeight(), 0.1, 400.0);
	int curFPS = 0;

	int updates = 0;
	//int totalUpdates = 0;
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
	const int s = 8;

	Chunk::init();

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

	while (!glfwWindowShouldClose(game->window))
	{
	start:
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


			if (xrotation > 3.14/2) xrotation = 3.14/2;
			if (xrotation < -3.14/2) xrotation = -3.14/2;

			fw.x = cos(yrotation) * cos(xrotation);
			fw.y = sin(xrotation);
			fw.z = cos(xrotation) * sin(-yrotation);

			if (forward) {
				eye += fw;
			}

			if (backward) {
				eye -= fw;
			}

			if (left) {
				eye -= fw.cross(up).normalize();
			}

			if (right) {
				eye += fw.cross(up).normalize();
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
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(eye[0], eye[1], eye[2], eye[0] + fw[0], eye[1] + fw[1], eye[2] + fw[2], up[0], up[1], up[2]);

		for (int x = 0; x < s; x++) {
			for (int y = 0; y < s; y++) {
				for (int z = 0; z < s; z++) {
					chunks[x][y][z]->render();
				}
			}
		}

		frames++;

		glfwSwapBuffers(game->window);
	}

end:
	glfwTerminate();
	return 0;
}