#pragma once
#include <vector>
#include "glm.hpp"

constexpr int LOD_SIZE = 4096 * 2;
// simple class with static functions that we will use to generate a poisson distribution within a certain area
class PoissonSampler {
public:
	static std::vector<glm::vec2> generatePoints(float sx, float sz, bool test=false, float dist=0);
	static std::vector<glm::vec2> testPoints(int num);
};