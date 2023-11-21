#include "PoissonSampler.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <random>
#include <cmath>
#include "World.h"
#include <iostream>
#include <algorithm>

// much inspiration for this implementation is based on coding train's implementation
// based off this material:
// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
// https://www.jasondavies.com/poisson-disc/
// https://editor.p5js.org/bggridley/sketches/bgUyGyzUb
// https://editor.p5js.org/codingtrain/sketches/4N78DFCXN


std::vector<glm::vec2> PoissonSampler::generatePoints(float sx, float sz, bool test, float dist) {
	std::vector<glm::vec2> points = std::vector<glm::vec2>();
	std::unordered_map<int, glm::vec2> grid = std::unordered_map<int, glm::vec2>();

	std::vector<glm::vec2> active = std::vector<glm::vec2>();
	std::vector<glm::vec2> ordered = std::vector<glm::vec2>();

	float w = 13 / sqrt(2); // controls grid size


	if (test) {
		w = dist / sqrt(2);
	}

	float length = LOD_SIZE;
	float width = LOD_SIZE;

	int cols = floor(length / w);
	int rows = floor(width / w);


	float x = length / 2;
	float z = width / 2;

	//std::cout << "staarting X: " << length << "z: " << width << std::endl;

	int i = floor(x / length);
	int j = floor(z / width);

	grid.emplace(i + j * cols, glm::vec2(x, z));
	active.push_back(glm::vec2(x, z));

	std::random_device rd;  // Obtain a random number from hardware
	std::mt19937 eng(rd()); // Seed the generator


	int numTimes = 1500 * 4;

	if (dist != 0) {
		numTimes = 15000;
	}

	for (int total = 0; total < numTimes; total++) {
		int activeSize = active.size();
		if (activeSize > 0) {
			std::uniform_real_distribution<> distr(w, w * 2.5); // Define the range [20, 60)
			std::uniform_real_distribution<> testdistr(dist, 60); // Define the range [20, 60)
			std::uniform_int_distribution<> index_dist(0, activeSize - 1);

			int randIndex = index_dist(eng);

			glm::vec2 pos = active[randIndex];

			bool found = false;
			int k = 10; // Assuming k is 10 as an example
			for (int n = 0; n < k; ++n) {
				glm::vec2 sample = glm::circularRand(1.0f); // Random 2D vector of length 1
				//std::cout << sample.x << ", " << sample.y << " | random direction" << std::endl;

				float m = distr(eng); // Random magnitude in the range [20, 60)

				if (test) {
					m = testdistr(eng);
				}

				sample = glm::normalize(sample) * m; // Set the magnitude to 'm', sample is already len 1
				sample += pos;

				
				float col = floor(sample.x / w);
				float row = floor(sample.y / w);

				float h = (World::getHeight(sx + sample.x, sz + sample.y) / 320.f); // 64 * 5 is height should use a constexpr for this
				// especially since the player's height in voxels is set to CHUNK_SIZE * chunk height


				int key = static_cast<int>(col + row * cols);

				if (col > -1 && row > -1 && !grid.count(key)) {
					bool ok = true;
					for (int i = -1; i <= 1; ++i) {
						for (int j = -1; j <= 1; ++j) {
							int index = (col + i) + (row + j) * cols; 
							int neighborKey = static_cast<int>(index);
							if (grid.count(neighborKey)) {
								glm::vec2 neighbor = grid[neighborKey];
								float d = glm::distance(sample, neighbor);


								if (test) {
									if (d < dist) {
										ok = false;
									}
								}
								else {
									if (d < (1 - h) * (1 - h) * (1 - h) * 120) {
										ok = false;
									}
								}
								
							}
						}
					}
					if (ok && sample.x < length && sample.y < width) {
						grid[key] = sample;
						active.push_back(sample);
						ordered.push_back(sample);

						//std::cout << sample.x << ", " << sample.y << std::endl;
						break;
					}
				}
			}




			// Use 'sample' for further operations
			// You can output 'sample' or perform additional operations here

		}
	}


	// Use the random device to seed the random number generator
	std::mt19937 g(rd());

	// Shuffle the vector using std::shuffle
	std::shuffle(ordered.begin(), ordered.end(), g);


	std::cout << ordered.size() << " is amount of points 256x256" << std::endl;


	 {
		int amnt = 16;
		int increment = LOD_SIZE / amnt;

		for (int i = 0; i < amnt; i++) {
			ordered.push_back(glm::vec2(0, i * increment));
			ordered.push_back(glm::vec2(LOD_SIZE, i * increment));

			ordered.push_back(glm::vec2(i * increment, 0));
			ordered.push_back(glm::vec2(i * increment, LOD_SIZE));
		}
	}
	 
	 
	 
	 
	return ordered;
}

