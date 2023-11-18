#pragma once
#include "Triangle.h"
#include "Primitives.h"

class DAG {
public:
	Triangle* root;
	DAG(Triangle* r) {
		this->root = r;
	}
	~DAG();

	void print(Triangle* tri = nullptr) {
		return;

		if (tri == nullptr) {
			tri = root;
		}

		if (tri != nullptr) {
			if (tri->children.size() == 0) {
				tri->print();
			}

			for (int i = 0; i < tri->children.size(); i++) {
				print(tri->children[i]);
			}
		}
	}


	std::vector<Triangle*> extract() {
		std::vector<Triangle*> triangles;

		extractStep(triangles, root);

		return triangles;
	}

	void extractStep(std::vector<Triangle*>& vec, Triangle* t) {
		if (!t->visited) {
			int sz = t->children.size();
			
			if (sz == 0) {
				t->visited = true;

				auto v = t->getVertices();
				for (int i = 0; i < 3; i++) {
					glm::vec2 p = v[i];

					if (p == root->edges[0]->p1 ||
						p == root->edges[1]->p1 ||
						p == root->edges[2]->p1) {
						return;
					}
				}



				vec.push_back(t);
				
			}
			else if (sz == 2) {
				t->visited = true;
				extractStep(vec, t->children[0]);
				extractStep(vec, t->children[1]);
			}
			else if (sz == 3) {
				t->visited = true;
				extractStep(vec, t->children[0]);
				extractStep(vec, t->children[1]);
				extractStep(vec, t->children[2]);
			}
		}
	}

// find triangle in another triangle
	Triangle* findTriangle(Triangle* cur, glm::vec2 p) {
		if (cur == nullptr) cur = root;

		std::vector<Triangle*> children = cur->children;
		size_t numChildren = children.size();

		if (numChildren == 0) {
			//std::cout << "0 chilren" << std::endl;
			// well, we have found the triangle!
			return cur;
		}
		else if (numChildren == 2) {
			//children[0]->print();
			//children[1]->print();
			if (Primitives::pointInTriangle(p, children[0]->getVertices())) {
				return findTriangle(children[0], p);
			}
			else {
				return findTriangle(children[1], p);
			}
		}
		else if (numChildren == 3) {
			if (Primitives::pointInTriangle(p, children[0]->getVertices())) {
				return findTriangle(children[0], p);
			}
			else if(Primitives::pointInTriangle(p, children[1]->getVertices())) {
				return findTriangle(children[1], p);
			}
			else {
				return findTriangle(children[2], p);
			}
		}


		return nullptr;
	}
};