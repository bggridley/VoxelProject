#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

class Game {
public:
	Game();
	~Game();
	
	GLFWwindow* window;
	void init(); // this is necessary so that textures can be created
	void update(float delta, int updateCount, int frameCount);
	int updates;
	void render(float alpha);
	void setDimensions(float width, float height);
	void setFullscreenDimensions(float width, float height);
	float getWidth();
	float getHeight();
	float getScale();
	enum GameState {
		MAIN_MENU,
		INGAME
	};

	bool zoomIn = false;
	bool zoomOut = false;
	bool mousePressed = false;
private:
	int state;
	int fps;
	float scale = 1.0f;
	float width;
	float height;
	float fullscreenWidth;
	float fullscreenHeight;
	bool vsync = true;
	bool fullscreen = false;
};