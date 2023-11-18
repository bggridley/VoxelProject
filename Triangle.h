#pragma once
#include "glm.hpp"
#include "Edge.h"
#include <vector>
// edges shall be defined as such

// vertex 0 -> 1 = edge 0
// vertex 1 -> 2 = edge 1
// vertex 2 -> 0 = edge 2

class Edge;

class Triangle {
public:
	bool visited = false;
	Edge* edges[3];
	std::vector<Triangle*> children;

	Triangle() {

	}

	Triangle(const Triangle &t) {
		children = std::vector<Triangle*>();
		children.reserve(2);

		for (int i = 0; i < 3; i++) {
			this->edges[i] = new Edge(*t.edges[i], this);
		}
	}

	Triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, bool boundary=false, Triangle* adjacentTriangle=nullptr) {
		children = std::vector<Triangle*>();

		// we need to condition this to ALWAYS maintain CCW order

		setEdges(p1, p2, p3, boundary);
	}

	void setRawEdges(Edge** edges) {
		this->edges[0] = edges[0];
		this->edges[1] = edges[1];
		this->edges[2] = edges[2];
	}

	void setEdges(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, bool boundary = false, Edge* sharedEdge = nullptr) {

		//if (Primitives::orientation(p1, p2, p3) == -1) {
		//	std::swap(p3, p1);
		//}

		//std::cout << "befoer EDGE FLIP" << std::endl;
		//print();

		//std::cout << "END" << std::endl << std::endl;

		//std::vector<Edge*> oldShared = std::vector<Edge*>();

		//for (int i = 0; i < 3; i++) {
		//	if (edges[i] != nullptr) {
		//		if (edges[i]->shared != nullptr) {
		//			oldShared.push_back(edges[i]->shared);
		//			//edges[i]->shared->shared = nullptr;
		//		}


		//		//delete edges[i];
		//		//edges[i] = nullptr;
		//	}
		//}

		edges[0] = new Edge(this, p1, p2, boundary);
		edges[1] = new Edge(this, p2, p3, boundary);
		edges[2] = new Edge(this, p3, p1, boundary);

		//for (int i = 0; i < 3; i++) {
		//	Edge* e = edges[i];
		//	for (int j = 0; j < oldShared.size(); j++) {
		//		Edge* s = oldShared[j];
		//		if (e->sameEdge(s)) {
		//			e->shared = s;
		//			s->shared = e;
		//			std::cout << "found a same edge" << std::endl;
		//		}
		//	}

		//	if (sharedEdge != nullptr && e->sameEdge(sharedEdge)) {
		//		e->shared = sharedEdge;
		//		sharedEdge->shared = e;
		//	}
		//}


	}
	
	/// here's what we need to do

	// find the triangle containing edge
	// p1 to q

	
	// stuff like this is really not that fast
	// but it's at least comprehensible
	// 9 checks, still linear

	void changeVertexTo(glm::vec2 start, glm::vec2 end, Triangle* other) {
		for (int i = 0; i < 3; i++) {
			Edge* e = edges[i];


			bool either = false;
			if (e->p1 == start) {
				either = true;
				e->p1 = end;
				e->shared = nullptr;
			}
			else if (e->p2 == start) {
				either = true;
				e->p2 = end;

				e->shared = nullptr;
			}



			if (either) { // meaning, we have changed the current edge....

			
				Edge* oldEdge = other->findEdge(e->p1, e->p2);

				if (oldEdge != nullptr && oldEdge->shared != nullptr) {
					//std::cout << "this is good" << std::endl;
					//e->shared = oldEdge;

				
					e->shared = oldEdge->shared;
					oldEdge->shared->shared = e;
					//std::cout << "not null!" << std::endl;
				}
			}
		}
	}

	Edge* findEdge(glm::vec2 p1, glm::vec2 p2) {
		for (int i = 0; i < 3; i++) {
			if (edges[i]->sameEdge(p1, p2)) {
				//std::cout << "find edge worked!" << std::endl;
				return edges[i];

			}
		}

		return nullptr;
	}

	glm::vec2 findNotOnEdge(Edge* e) {
		glm::vec2 p1 = e->p1;
		glm::vec2 p2 = e->p2;
		for (int i = 0; i < 3; i++) {
			glm::vec2 e = edges[i]->p1;

			if (e != p1 && e != p2) {
				//std::cout << "found " << e.x << ", " << e.y << std::endl;
				return e;
			}

		
		}
	}

	void print() {
		std::cout << "Triangle" << std::endl;
		for (int i = 0; i < 3; i++) {
			if (edges[i] == nullptr) continue;
			std::cout << "	Edge " << i << " (" << edges[i]->p1.x << " " << edges[i]->p1.y << "), (" << edges[i]->p2.x << ", " << edges[i]->p2.y << ")" << std::endl;

			if (edges[i]->shared != nullptr) {
				std::cout << "		Shared w/ Edge " << i << " (" << edges[i]->shared->p1.x << " " << edges[i]->shared->p1.y << "), (" << edges[i]->shared->p2.x << ", " << edges[i]->shared->p2.y << ")" << std::endl;
			}
		}

		std::cout << std::endl;
	}

	// make sure we cache this for faster accesss
	// dont need to do this every single time
	std::vector<glm::vec2> getVertices() {
		std::vector<glm::vec2> verts = std::vector<glm::vec2>();
		verts.push_back(edges[0]->p1);
		verts.push_back(edges[1]->p1);
		verts.push_back(edges[2]->p1);
		return verts;
	}

	~Triangle() {
		free(edges);
	}

private:

};