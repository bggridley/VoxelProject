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

	void print(Triangle* tri = nullptr, std::string indent = std::string("")) {
		if (tri != nullptr) {
			

			tri->print(indent);

			indent = indent + "---";

			for (int i = 0; i < tri->children.size(); i++) {
				print(tri->children[i], indent);
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

				bool add = true;
				auto v = t->getVertices();
				for (int i = 0; i < 3; i++) {
					glm::vec2 p = v[i];

					if (p == root->edges[0]->p1 ||
						p == root->edges[1]->p1 ||
						p == root->edges[2]->p1) {
						add = false;

			
						break;
					}
				}


				if (add) {
					vec.push_back(t);
				}
				
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
		std::vector<Triangle*> children = cur->children;
		size_t numChildren = children.size();

		if (numChildren == 0) {
			//std::cout << "0 chilren" << std::endl;
			// well, we have found the triangle!
			if (Primitives::pointInTriangle(p, cur->getVertices())) {
				return cur;
			}
			else {
				std::cout << "IDK how this happened. this is fugged up" << std::endl;
			}
			
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