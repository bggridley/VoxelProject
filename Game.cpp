#include "Game.h"

Game::Game() {
}

void Game::init() {
	//ssetState(GameState::MAIN_MENU);
}

void Game::update(float delta, int updateCount, int fps) {
	this->fps = fps;
	this->updates = updateCount;

	if (state == GameState::MAIN_MENU) {

	}

	else if (state == GameState::INGAME) {
		int midX = getWidth() / 2;
		int midY = getHeight() / 2;

		if (zoomOut) {
			scale -= 0.1 * delta;

			if (scale < 0.85f) {
				scale = 0.85f;
			}
		}

		if (zoomIn) {
			scale += 0.1 * delta;

			if (scale > 2.0f) {
				scale = 2.0f;
			}

		}

	
	}
}

void Game::render(float alpha) {
	std::string framespersec = "FPS:" + std::to_string(fps);
}

void Game::setDimensions(float width, float height) {
	this->height = height;
	this->width = width;
}

void Game::setFullscreenDimensions(float width, float height) {
	this->fullscreenWidth = width;
	this->fullscreenHeight = height;
}

float Game::getWidth() {
	return fullscreen ? fullscreenWidth : width;
}

float Game::getHeight() {
	return fullscreen ? fullscreenHeight : height;
}

float Game::getScale() {
	return scale;
}