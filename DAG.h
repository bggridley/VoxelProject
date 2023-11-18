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

// find triangle in another triangle
	Triangle* findTriangle(Triangle* cur, glm::vec2 p) {
		if (cur == nullptr) cur = root;

		std::vector<Triangle*> children = cur->children;
		size_t numChildren = children.size();

		if (numChildren == 0) {
			std::cout << "0 chilren" << std::endl;
			// well, we have found the triangle!
			return cur;
		}
		else if (numChildren == 2) {
			children[0]->print();
			children[1]->print();
			children[2]->print();
			if (Primitives::pointInTriangle(p, children[0]->getVertices())) {
				return findTriangle(children[0], p);
			}
			else {
				return findTriangle(children[1], p);
			}
		}
		else if (numChildren == 3) {
			std::cout << "123 chilren" << std::endl;
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