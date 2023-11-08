#pragma once
#include <vector>
#include "glm.hpp"

// simple class with static functions that we will use to generate a poisson distribution within a certain area
class PoissonSampler {
public:
	static std::vector<glm::vec2> generatePoints(float sx, float sz, float ex, float ez);
};