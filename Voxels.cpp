#include <iostream>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Chunk.h"
#include <stdlib.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <iterator>
#include <unordered_map>
#include "gtx/hash.hpp"
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <sstream>
#include <filesystem>
#include <string>
#include <chrono>
#include <stack>
#include "VerticalLine.h"
#include "PoissonSampler.h"
#include "Primitives.h"
#include "BackgroundChunk.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


bool left = false, right = false, forward = false, backward = false, space = false, shift = false;
float xrotation = 0;
float yrotation = 0;
bool vsync = true;
double mouseX, mouseY;
int totalUpdates = 0;
bool wireframe = false;
const int s = 4; // dimension of chunk grid
GLFWwindow* window;
//GLFWwindow* shared_window;
bool fullscreen = false;
std::unordered_map<glm::vec3, Chunk*> chunks;
std::queue <std::pair<glm::vec3, Chunk*>> chunkQueue; // avoid weird synchronization issues // contains just the vec3's so that chunks doesn't have to be searched through...
std::queue <glm::vec3> chunkDequeue; // avoid weird synchronization issues // contains just the vec3's so that chunks doesn't have to be searched through...

float distanceMoved = 0;
std::timed_mutex chunkMutex;

glm::vec3 velocity;

int chunkLoadRange = 5;

GLuint texture1;

struct Plane {
	glm::vec3 pos;
	glm::vec3 dir;
};


glm::vec3 fw = { 1.0, 0.0, 0.0 };
glm::vec3 eye = { 0.0, 300, 0.0 };
glm::vec3 up = { 0.0, 1.0, 0.0 };

/*
*
	gl_Position.z = 2.0*log(gl_Position.w*C + 1)/log(far*C + 1) - 1;
	gl_Position.z *= gl_Position.w;
*/

// Shaders
const GLchar* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 aTex;\n"
"uniform mat4 projectionMatrix;\n"
"uniform mat4 viewMatrix;\n"
"uniform mat4 modelViewMatrix;\n"
//"layout (location = 1) in vec3 color;\n"
"out vec3 light;\n"
"out vec2 tex;\n"
"void main()\n"
"{\n"
"gl_Position = projectionMatrix * viewMatrix * modelViewMatrix * vec4(position, 1.0);\n"
"vec3 mvPos = vec3(modelViewMatrix * vec4(position.xyz, 1.0));\n"
"light = vec3(vec4(mvPos.xyz, 1.0) / 256);\n"
"tex = aTex;\n"
"}\0";
const GLchar* fragmentShaderSource =
"#version 330 core\n"
"in vec3 light;\n"
"in vec2 tex;\n"
"out vec4 color;\n"
"uniform sampler2D materials;\n"
"void main()\n"
"{\n"
"color = vec4((texture(materials, tex).rgb + light * 2) / 3.0, 1.0);\n"
"}\n\0";

bool frustumCulling(glm::vec3 chunkPos, Plane farPlane, Plane leftPlane, Plane rightPlane, Plane topPlane, Plane bottomPlane) {
	return glm::dot(farPlane.dir, chunkPos - farPlane.pos) >= 0
		&& glm::dot(leftPlane.dir, chunkPos - leftPlane.pos) >= 0
		&& glm::dot(rightPlane.dir, chunkPos - rightPlane.pos) >= 0
		&& glm::dot(topPlane.dir, chunkPos - topPlane.pos) >= 0
		&& glm::dot(bottomPlane.dir, chunkPos - bottomPlane.pos) >= 0;
}

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
		case GLFW_KEY_F:
			//game->toggleFullscreen();
			if (val) {
				fullscreen = !fullscreen;
				GLFWmonitor* monitor = glfwGetPrimaryMonitor();
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);

				if (fullscreen) {
					glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
					glViewport(0, 0, mode->width, mode->height);
					glfwSwapInterval(vsync);
				}
				else {
					glfwSetWindowMonitor(window, NULL, (mode->width - mode->width / 1.5) / 2, (mode->height - mode->height / 1.5) / 2, mode->width / 1.5, mode->height / 1.5, 0);
					glViewport(0, 0, mode->width / 1.5, mode->height / 1.5);
					glfwSwapInterval(vsync);
				}
			}

			break;
		}
	}
}

int main(int argc, char** argv) {

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


	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	window = glfwCreateWindow(mode->width / 1.5, mode->height / 1.5, "Voxels", NULL, NULL);

	glfwSetWindowPos(window, (mode->width - mode->width / 1.5) / 2, (mode->height - mode->height / 1.5) / 2);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, keyboard);
	glfwMakeContextCurrent(window);

	float aspect = mode->width / mode->height;

	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to intitialize GLEW" << std::endl;
	}
	else {
		std::cout << "GLEW intiailized successfully" << std::endl;
	}

	glViewport(0, 0, mode->width / 1.5, mode->height / 1.5);


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

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);



	int texWidth, texHeight, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.


	std::string s = "/texture.png"; // fix basolute paht later
	unsigned char* data = stbi_load(s.c_str(), &texWidth, &texHeight, &nrChannels, 0);
	if (data)
	{
		std::cout << "loaded texture." << std::endl;//uniform sampler2D ourTexture;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}



	//std::cout << "TEST RESULTS NOW " << std::endl;

	///glm::vec2 p3(3, 3);
	//glm::vec2 p2(4, 4);
	//glm::vec2 p1 (5, 3);

	//glm::vec2 testPoint(4.5, 3.3);


	//std::cout << "TEST" << Primitives::pointInTriangle(testPoint, p3, p1, p2);


	BackgroundChunk* c = new BackgroundChunk(); // does this evne work?
	std::cout << "Wtf " << std::endl;
	c->init();

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, game->getWidth() / game->getHeight(), 0.1, 400.0);
	*/

	float near = 0.1f;
	float far = 2000.f;
	float fov = 60.0f;
	glm::mat4 projection = glm::perspective(glm::radians<float>(fov), (float)mode->width / (float)mode->height, near, far);

	int curFPS = 0;
	int updates = 0;
	int frames = 0;
	const int fps = 60;
	float t = 0.0;

	float dt = 1.0f / 60.0f; // target ups regardless of fps
	float deltaTime = 0.0f;
	float currentTime = glfwGetTime(); // gotta do this so it doesnt blow up
	float accumulator = 0.0f;

	float rot = 0.0f;

	// pass this by value because it makes no sense to pass by reference if shaderprogram is on the stack....
	Chunk::init(shaderProgram);
	VerticalLine::init();
	

	// poisson points
	std::vector<glm::vec2> poissonPoints = PoissonSampler::generatePoints(-CHUNK_SIZE * 4, -CHUNK_SIZE * 4, -CHUNK_SIZE * 2, -CHUNK_SIZE * 2);

	std::string a = "helo";
	std::string b = "hi";

	//(a + b).length() ? > 120;


	auto chunkThreadBody = []() {
		std::vector<glm::vec3> chunkList;

		auto addChunk = [&chunkList](int chunkX, int chunkY, int chunkZ) {
			glm::vec3 chunkCoords = glm::vec3(chunkX, chunkY, chunkZ);


			if (std::find(chunkList.begin(), chunkList.end(), chunkCoords) == chunkList.end()) {
				chunkList.push_back(chunkCoords);
				chunkQueue.push(std::pair<glm::vec3, Chunk*>(chunkCoords, new Chunk(chunkX * CHUNK_SIZE, chunkY * CHUNK_SIZE, chunkZ * CHUNK_SIZE)));
			}
		};

		int lastXAttempt = eye.x;
		int lastZAttempt = eye.z;

		bool started = false;
		int maxSize = 9;
		int chunkHeight = 5;
		float maxSizeSqrt = maxSize * sqrt(1.8);
		int ticker = 0;

		while (!glfwWindowShouldClose(window)) {

			//std::cout << distanceMoved << std::endl;

				int wc = 0; // working chunks

				float eyeX = eye.x;
				float eyeZ = eye.z;
				if (chunkQueue.empty() && chunkDequeue.empty()) {
					ticker++;
					if (ticker % 2 == 0) {
						//std::cout << "huh" << std::endl;
						// only do a new batch of chunks if the queue is fully processed...
						if (!started) started = true;


						for (int i = 0; i < chunkHeight; i++) {
							addChunk(eyeX / CHUNK_SIZE, i, eyeZ / CHUNK_SIZE);
							wc++;
						}



						for (int i = 0; i < maxSize; i++) {
							int x = -i; // bottom left corner...
							int z = -i;

							int xDir = 1;
							int zDir = 0;

							for (int j = 0; j < 4; j++) {
								int lastX = x;
								int lastZ = z;


								x += xDir * i * 2;
								z += zDir * i * 2;

								int dir = xDir;
								int start = lastX;
								int check = x;

								if (zDir != 0) {
									dir = zDir;
									start = lastZ;
									check = z;
								}

								for (int cur = start; cur != check; cur += dir) {
									int chunkX = zDir != 0 ? lastX : cur;

									int chunkZ = zDir != 0 ? cur : lastZ;
									for (int chunkY = 0; chunkY < chunkHeight; chunkY++) {

										addChunk(eyeX / CHUNK_SIZE + chunkX, chunkY, eyeZ / CHUNK_SIZE + chunkZ);
										wc++;
									}
								}

								if (xDir == 1) {
									zDir = 1;
									xDir = 0;
								}
								else if (zDir == 1) {
									xDir = -1;
									zDir = 0;
								}
								else if (xDir == -1) {
									zDir = -1;
									xDir = 0;
								}
								else if (zDir == -1) {
									xDir = 1;
									zDir = 0;
								}
							}
						}

					}
					else {

						if (!started) started = true;

						std::vector<glm::vec3>::iterator it;
						for (it = chunkList.begin(); it != chunkList.end();) {
							glm::vec3 c = *it;
							//std::cout << "c: " << c.x << std::endl;
							if (glm::distance(glm::vec3(c.x, 0, c.z) * (float)CHUNK_SIZE, glm::vec3(eyeX, 0, eyeZ)) > maxSizeSqrt * CHUNK_SIZE) {
								chunkDequeue.push(c);
								it = chunkList.erase(it);
								wc++;
							}
							else {
								it++;
							}

							if (wc > 200) {
								break;
							}
						}
					}
				}


		}
	};

	std::thread chunk_thread(chunkThreadBody);


	//std::cout << "Total Triangles: " << Chunk::totalTriangles << std::endl;

	glUseProgram(shaderProgram);
	GLuint projectionID = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(glGetUniformLocation(shaderProgram, "materials"), 0); // 0th location


	GLuint viewID = glGetUniformLocation(shaderProgram, "viewMatrix");
	GLuint modelViewID = glGetUniformLocation(shaderProgram, "modelViewMatrix");

	std::cout << "ID::" << modelViewID << std::endl;


	//float a = 5.51;
	//int b = static_cast<int>(a);
	//std::cout << b << std::endl;

	std::chrono::duration<double> totalTime = std::chrono::duration<double>::zero();
	int counter = 0;

	while (!glfwWindowShouldClose(window))
	{
		if (t >= 1.0f) {
			t -= 1.0f;

			double avg = 0.0;
			
			if (counter != 0) {
				avg = totalTime.count() / (double)counter;

			}

			double avgChunk = 0.0;
				if (Chunk::totalCount != 0) {
					avgChunk = Chunk::totalTime.count() / Chunk::totalCount;
				}
			//std::ostringstream os;
			std::cout << "FPS: " << frames << " | UPS: " << updates << " | CHUNKS: " << chunks.size() << " | GPU LD: " << avg << "s | AVG GEN: " << avgChunk << "s" << std::endl;
			std::cout << "POS:" << eye.x << ", " << eye.z << std::endl;
			
			//glfwSetWindowTitle(window, os.str().c_str());
			curFPS = frames;
			frames = 0;
			updates = 0;
			//os.clear();
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
			glfwGetCursorPos(window, &mx, &my);

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

			glm::vec3 eyeOld = eye;

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

			//velocity.y += -1.f / fps; // velocity changes at 9.8 m/s

			//eye.y += velocity.y;

			//glm::vec3 feetPos = eye - glm::vec3(0, 2.1f, 0);
			//glm::vec3 feetChunk = glm::vec3((int)feetPos.x / CHUNK_SIZE, (int)(feetPos.y) / CHUNK_SIZE, (int)feetPos.z / CHUNK_SIZE);

			////Chunk cur = std::find(chunks.begin(), chunks.end(), eyeChunk);
			//if (chunks.find(feetChunk) != chunks.end()) { // chunk exists at that pos
			//	Chunk* c = chunks.at(feetChunk);

			//	int x = (int)feetPos.x % CHUNK_SIZE;
			//	int y = (int)feetPos.y % CHUNK_SIZE;
			//	int z = (int)feetPos.z % CHUNK_SIZE;

			//	//if (c->materials[x][y][z]) {
			//		// STOP, onnGROUND
			//	//	eye.y = (feetChunk.y * CHUNK_SIZE) + y + 3;
			//		//velocity.y = 0;

			//	//}



			//}

			//if (space) {
			//	eye += up;
			//}

			//if (shift) {
			//	eye -= up;
			//}




			distanceMoved += glm::distance(eye, eyeOld);


			t += dt;
			accumulator -= dt;
			rot += dt;

			int chunksPerUpdate = 10;
			for (int i = 0; i < chunksPerUpdate; i++) {
				//if (chunkQueue.empty() && chunkDequeue.empty()) break;


				if (!chunkQueue.empty()) {
					//chunkMutex.lock();
					std::pair<glm::vec3, Chunk*> pair = chunkQueue.front();


					if (chunks.find(pair.first) != chunks.end()) {
						std::cout << "sent a duplicate chunk it. will break later." << std::endl;
					}

					auto start = std::chrono::high_resolution_clock::now();


					Chunk* c = pair.second;

					glGenVertexArrays(1, &(c->vao));
					glBindVertexArray(c->vao);


					glGenBuffers(1, &(c->iID));
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->iID);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, c->indices.size() * sizeof(GL_UNSIGNED_INT), c->indices.data(), GL_DYNAMIC_DRAW);

					glBindBuffer(GL_ARRAY_BUFFER, c->vID); // bind the vertices

					glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(GL_UNSIGNED_INT), (GLvoid*)0);
					glEnableVertexAttribArray(0);


					glBindBuffer(GL_ARRAY_BUFFER, c->tID);
					glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)0);

					glEnableVertexAttribArray(1);

					glBindVertexArray(0); // Unbind VAO

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					glBindBuffer(GL_ARRAY_BUFFER, 0);

					chunks.emplace(pair.first, c);
					chunkQueue.pop(); // delete the item that was just inserted into chunks

					auto end = std::chrono::high_resolution_clock::now();

					totalTime += (end - start);
					counter++;
				}


				if (!chunkDequeue.empty()) {
					glm::vec3 c = chunkDequeue.front();

					//delete chunks.at(c);
					if(chunks.find(c) != chunks.end()) {
					Chunk* chunk = chunks.at(c);
					chunks.erase(c);
					


					//	if(c)
					delete chunk;
					}
					chunkDequeue.pop();
				}

			}

		}

		// render

		float alpha = accumulator / dt;

		glm::mat4 view = glm::lookAt(
			eye,
			(eye + fw),
			glm::vec3(0, 1, 0)
		);



		glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

		//glfwMakeContextCurrent(window);
		glClearColor(0.5f, 0.5f, 1.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);



		float farDist = far + CHUNK_SIZE;
		const float midVertical = farDist * tanf((fov) * (3.1415926f / 180.f));
		const float midHorizontal = midVertical * aspect;

		glm::vec3 eyeRight = normalize(cross(fw, up));
		glm::vec3 eyeUp = normalize(cross(eyeRight, fw));

		glm::vec3 fwFar = normalize(fw) * farDist;


		//std::cout << eyeUp.x << ", " << eyeUp.y << ", " << eyeUp.z << std::endl;



		//Plane nearPlane = {eye + near * fw, fw};
		Plane farPlane = { eye + fwFar, -fw };
		Plane leftPlane = { eye, glm::cross((fwFar)-(eyeRight * midHorizontal), eyeUp) };
		Plane rightPlane = { eye, glm::cross(eyeUp, (fwFar)+(eyeRight * midHorizontal)) };
		Plane topPlane = { eye, glm::cross(eyeRight, (fwFar)-(eyeUp * midVertical)) };
		Plane bottomPlane = { eye, glm::cross((fwFar)+(eyeUp * midVertical), eyeRight) };


		glm::vec3 eyeChunkPos = glm::vec3((int)eye.x - 1 / CHUNK_SIZE, (int)eye.y / CHUNK_SIZE, (int)eye.z / CHUNK_SIZE);
		//glm::vec3 leftPlane = eye;

		int draw = 0, skip = 0;
		std::unordered_map<glm::vec3, Chunk*>::iterator it;
		for (it = chunks.begin(); it != chunks.end(); it++)
		{
			glm::vec3 chunkPos = it->first;


			//std::cout << chunkPos.x << ", " << chunkPos.y << ", " << chunkPos.z << std::endl;


			//if (curChunk) std::cout << "egg" << std::endl;
			chunkPos *= CHUNK_SIZE;
			bool curChunk = glm::distance(eye, chunkPos) <= CHUNK_SIZE * 1.73205080757;

			//if (curChunk) std::cout << "CURCHUNK" << std::endl;

			glm::vec3 chunkZ = chunkPos + glm::vec3(0, 0, CHUNK_SIZE);
			glm::vec3 chunkX = chunkPos + glm::vec3(CHUNK_SIZE, 0, 0);
			glm::vec3 chunkXYZ = chunkPos + glm::vec3(CHUNK_SIZE, 0, CHUNK_SIZE);

			/*glm::vec3 chunkY = chunkPos + glm::vec3(0, CHUNK_SIZE, 0);

			glm::vec3 chunkZY = chunkPos + glm::vec3(0, CHUNK_SIZE, CHUNK_SIZE);
			glm::vec3 chunkXY = chunkPos + glm::vec3(CHUNK_SIZE, CHUNK_SIZE, 0);
			glm::vec3 chunkXYZ = chunkPos + glm::vec3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);*/

			// y is irrelevant right now

			//glm::vec3 n = (fw);
			///glm::vec3 p = (eye);

			// direction, point - eye

			draw++;
			if (!it->second->empty && (curChunk
				|| frustumCulling(chunkPos, farPlane, leftPlane, rightPlane, topPlane, bottomPlane)
				|| frustumCulling(chunkXYZ, farPlane, leftPlane, rightPlane, topPlane, bottomPlane)
				|| frustumCulling(chunkZ, farPlane, leftPlane, rightPlane, topPlane, bottomPlane)
				|| frustumCulling(chunkX, farPlane, leftPlane, rightPlane, topPlane, bottomPlane))) {
				Chunk* c = (Chunk*)it->second;

					glUniformMatrix4fv(modelViewID, 1, GL_FALSE, glm::value_ptr(c->modelView));

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texture1);

					glBindVertexArray(c->vao);
					glDrawElements(GL_TRIANGLES, c->indicesCount, GL_UNSIGNED_INT, (void*)0);
					glBindVertexArray(0);

				
			}
			else {
				//skip++;
			}



		}


		for (const glm::vec2 &num: poissonPoints) {
			VerticalLine::render(modelViewID, num.x, num.y);
		}



		

		//	std::cout << draw << ", " << skip << std::endl;


		glDisable(GL_CULL_FACE);

		frames++;
		glfwSwapBuffers(window);
	}

	std::cout << "waiting for chunk thread to terminate" << std::endl;
	chunk_thread.join();

	glfwTerminate();
	return 0;
}
