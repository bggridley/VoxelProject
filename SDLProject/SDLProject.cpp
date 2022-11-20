#include <iostream>
#include <algorithm>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Game.h"
#include "Mesh.h"
#include "Chunk.h"
#include <stdlib.h>
#include "Vector3.h"

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "SDL failed to initialize" << std::endl;
	}
	else {
		std::cout << "SDL initialized successfully" << std::endl;
	}

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}
	else {
		std::cout << "SDL_Image intiialized successfully" << std::endl;
	}


	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		std::cout << "SDL_Mixer failed to intiialize" << std::endl;
	}
	else {
		std::cout << "SDL_Mixer initialized successfully" << std::endl;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
//s	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8); // antialiasing

	int display_count = 0, display_index = 0, mode_index = 0;
	SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

	if ((display_count = SDL_GetNumVideoDisplays()) < 1) {
		SDL_Log("SDL_GetNumVideoDisplays returned: %i", display_count);
		return 1;
	}

	if (SDL_GetDisplayMode(display_index, mode_index, &mode) != 0) {
		SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
		return 1;
	}

	SDL_CaptureMouse(SDL_TRUE);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	Game* game = new Game(); // create a new game instance
	std::cout << mode.w << " , " << mode.h << std::endl;
	game->setDimensions(mode.w / 1.5, mode.h / 1.5);
	game->setFullscreenDimensions(mode.w, mode.h);
	game->window = SDL_CreateWindow("Voxel Jawn", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, game->getWidth(), game->getHeight(), SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SDL_SetWindowFullscreen(game->window, 0);
	SDL_GLContext Context = SDL_GL_CreateContext(game->window);

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
	bool vsync = true;

	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, game->getWidth() / game->getHeight(), 0.1, 400.0);



	//glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_COLOR_MATERIAL);

	// done once, give us a fresh projection matrix.

	//glDisable(GL_DEPTH_TEST);



	SDL_GL_SetSwapInterval(vsync);

	//game->init();

	int curFPS = 0;

	int updates = 0;
	int totalUpdates = 0;
	int frames = 0;
	const int fps = 60;
	float t = 0.0;

	float dt = 1.0f / 60.0f; // TICKINTERVAL
	float deltaBuffer = 0.0f;
	float deltaTime = 0.0f;
	Uint64 currentTime = SDL_GetPerformanceCounter(); // gotta do this so it doesnt blow up
	float accumulator = 0.0f;

	float rot = 0.0f;
	//Chunk* chunk = new Chunk(0, 0, 0);

	bool left = false, right = false, forward = false, backward = false, space = false, shift = false;
	float xrotation = 0;
	float yrotation = 0;

	const int s = 14;

	Chunk::init();

	Chunk* chunks[s][s][s];

	GLuint cID = 0;
	glGenBuffers(1, &cID);

	//Mesh::fill(cID, GL_ARRAY_BUFFER)


	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				chunks[x][y][z] = new Chunk(x * CHUNK_SIZE, y * CHUNK_SIZE, z * CHUNK_SIZE);
			}
		}
	}

	while (true)
	{
	start:

		if (t >= 1.0f) {
			t -= 1.0f;
			std::cout << "FPS: " << frames << " | UPS: " << updates << std::endl;
			curFPS = frames;
			frames = 0;
			updates = 0;
		}

		Uint64 newTime = SDL_GetPerformanceCounter();
		deltaTime = float(newTime - currentTime) / SDL_GetPerformanceFrequency();
		currentTime = newTime;
		//std::cout << deltaTime << std::endl;
		accumulator += deltaTime;


		//std::cout << "delta: " << deltaTime << std::endl;
	
		/*
		GLfloat* vertices = (GLfloat*) malloc(sizeof(GLfloat) * 12);

		vertices[0] = -100;
		vertices[1] = 0;
		vertices[2] = 100;
		vertices[3] = -100;
		vertices[4] = 0;
		vertices[5] = -100;
		vertices[6] = 100;
		vertices[7] = 0;
		vertices[8] = 100;
		vertices[9] = 100;
		vertices[10] = 0;
		vertices[11] = -100;
		
		GLfloat* colors = (GLfloat*)malloc(sizeof(GLfloat) * 12);


		for (int i = 0; i < 12; i++) {
			colors[i] = 0.125f;
		}
		//indicesCount = 6;
		GLuint* indices = (GLuint*) malloc(sizeof(GLuint) * 6);/// = { 0, 1, 3, 3, 2, 0 };

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 3;
		indices[3] = 3;
		indices[4] = 2;
		indices[5] = 0;

		//Mesh* mesh = new Mesh(vertices, 12, colors, 12, indices, 6);

		*/
		

	//	std::cout << "HAPPENS:" << std::endl;

		
		while (accumulator >= dt) {
			//scale -= 0.001f;
			updates++;
			totalUpdates++;

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{

					goto end;
				}
				else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
					bool val = (e.type == SDL_KEYDOWN);
					switch (e.key.keysym.sym) {
					case SDLK_a:
						left = val;
						break;
					case SDLK_d:
						right = val;
						break;
					case SDLK_w:
						forward = val;
						break;
					case SDLK_s:
						backward = val;
						break;
					case SDLK_SPACE:
						space = val;
						break;
					case SDLK_LSHIFT:
						shift = val;
						break;
					case SDLK_v:
						if (val) {
							vsync = !vsync;
							SDL_GL_SetSwapInterval(vsync);
						}
						break;
					}
				}
				else if (e.type == SDL_MOUSEMOTION && totalUpdates > 60) {
					yrotation -= e.motion.xrel * 0.001;
					xrotation -= e.motion.yrel * 0.001;

					//std::cout << xrotation << std::endl;
				}
			}

			if (xrotation > 3.14/2) xrotation = 3.14/2;
			if (xrotation < -3.14/2) xrotation = -3.14/2;

			fw.x = cos(yrotation) * cos(xrotation);
			fw.y = sin(xrotation);
			fw.z = cos(xrotation) * sin(-yrotation);


			//fw.normalize();
			//glColor3f(1.0, 1.0, 1.0);
			//mesh->render();
			//game->update(dt, totalUpdates, curFPS);
			//std::cout << fw.x << ", " << fw.y << ", " << fw.z << std::endl;
		

			if (forward) {
				// move eye in the direction of forward...
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
			// process input with every update
			rot += dt;
		}


		float alpha = accumulator / dt;


		//glScalef(scale, scale, 1.0f);
	
	
	
			//glViewport(0, 0, game->getWidth(), game->getWidth());
		glEnable(GL_DEPTH_TEST);
		
		

		glClearColor(0.5f, 0.5f, 1.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		// render here


		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(eye[0], eye[1], eye[2], eye[0] + fw[0], eye[1] + fw[1], eye[2] + fw[2], up[0], up[1], up[2]);

	

		
		//std::cout << "EYE0" << (eye[0] + fw[0]) << std::endl;

		
		//gluLookAt(eye[0], eye[1], eye[2], 0.0, 0.0, 0.0, 0.0, 0.1, 0.0);
	

		//glRotatef(rot * 10, 0, 1, 0);
		//mesh->render();
		
		for (int x = 0; x < s; x++) {
			for (int y = 0; y < s; y++) {
				for (int z = 0; z < s; z++) {
					chunks[x][y][z]->render();
				}
			}
		}

		//glPopMatrix();
		//glMatrixMode()



		//glLoadIdentity();
		//glPushMatrix();
		
		//mesh->render();

		//game->render(alpha);
		frames++;

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

	

		SDL_GL_SwapWindow(game->window);
		//SDL_RenderPresent(game->renderer);
	}

end:

	SDL_DestroyWindow(game->window);
	Mix_Quit();
    IMG_Quit();
	SDL_Quit();
	return 0;
}