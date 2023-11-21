#pragma once
#include <glm/glm.hpp>
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
			Edge* oldEdgeShared = t.edges[i]->shared;

			this->edges[i] = new Edge(*t.edges[i], this);

			if (oldEdgeShared != nullptr) {
				oldEdgeShared->shared = this->edges[i];
			}
		}
	}

	Triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, bool boundary=false, Triangle* adjacentTriangle=nullptr) {
		children = std::vector<Triangle*>();

		setEdges(p1, p2, p3, boundary);
	}

	void setRawEdges(Edge** edges) {
		this->edges[0] = edges[0];
		this->edges[1] = edges[1];
		this->edges[2] = edges[2];
	}

	void setEdges(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, bool boundary = false, Edge* sharedEdge = nullptr) {

		if (Primitives::orientation(p1, p2, p3) == -1) {
			std::cout << "orientation swap. this will never print though :)" << std::endl;
			std::swap(p3, p1);
		}

		edges[0] = new Edge(this, p1, p2, boundary);
		edges[1] = new Edge(this, p2, p3, boundary);
		edges[2] = new Edge(this, p3, p1, boundary);
	}


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
		
					oldEdge->shared->shared = e;	
					e->shared = oldEdge->shared;
					
					//std::cout << "not null!" << std::endl;
				}
			}
		}

	}

	Edge* findEdge(Edge* e) {
		return findEdge(e->p1, e->p2);
	}

	// Not great but still linear
	// A better construction of Triangle and Edge classes
	// Could avoid constant searching like this
	// Between two triangles that result in 9 calls each time
	// Although, that's really not that bad...

	// Room for improvement though.

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
				return e;
			}
		}
	}

	void print(std::string indent = std::string("")) {
		std::cout << indent << "Triangle " << this << " | " << children.size() << " children" << std::endl;
		for (int i = 0; i < 3; i++) {
			if (edges[i] == nullptr) continue;
			std::cout << indent << "	Edge " << i << " (" << edges[i]->p1.x << " " << edges[i]->p1.y << "), (" << edges[i]->p2.x << " " << edges[i]->p2.y << ")" << std::endl;

			if (edges[i]->shared != nullptr) {
				std::cout << indent << "		Shared w/ Edge " << i << " (" << edges[i]->shared->p1.x << " " << edges[i]->shared->p1.y << "), (" << edges[i]->shared->p2.x << " " << edges[i]->shared->p2.y << ")" << std::endl;
				std::cout << indent << "		Incident on Tri: " << edges[i]->shared->triangle << std::endl;

			}
		}

		std::cout << std::endl;
	}

	// Could store these instead of having to generate them each time
	// But I really don't think it's an issue on modern hardware

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