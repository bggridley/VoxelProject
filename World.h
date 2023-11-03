#pragma once

constexpr float worldScale = 800.0f;

class World {
public:
	static float getHeight(float x, float z);
	static float getGrad(float x, float z);
};