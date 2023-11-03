#include "World.h"
#include "SimplexNoise.h"
#include <math.h>
#include "Chunk.h"


const int S = 120;
const int A = 36;
const int B = 48;
const int C = 36;
const int D = 36;
const int E = 4;
const int F = 2;

float h = 1e6;

float World::getHeight(float x, float z) {
	return 120
		+ A * ((SimplexNoise::noise(x / worldScale, z / worldScale)) + 0.25)
		+ B * ((SimplexNoise::noise(x / worldScale * 1.25, z / worldScale * 1.25)) + 0.5)
		+ C * ((SimplexNoise::noise(x / worldScale * 2, z / worldScale * 2)) - 0.5)
		+ D * ((SimplexNoise::noise(x / worldScale * 4, z / worldScale * 4)) + 0.25)
		+ E * ((SimplexNoise::noise(x / worldScale * 12, z / worldScale * 12)))
		+ F * ((SimplexNoise::noise(x / worldScale * 24, z / worldScale * 24)));
}


float World::getGrad(float x, float z) {
	float grad_x = (getHeight(x + h, z) - getHeight(x - h, z)) / (2 * h);
	float grad_z = (getHeight(x, z + h) - getHeight(x, z - h)) / (2 * h);

	return grad_x, grad_z;
}

