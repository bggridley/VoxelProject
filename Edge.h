#pragma once
#include "Triangle.h"

class Triangle;

class Edge {
public:
	Triangle* triangle;
	Edge* shared = nullptr;
	glm::vec2 p1, p2;
	bool boundary = false;
	Edge() {
		// never ever use this
	}

	Edge(const Edge& edge, Triangle* t) {
		triangle = t;
		shared = edge.shared;
		p1 = edge.p1;
		p2 = edge.p2;
		boundary = edge.boundary;
	}

	Edge(Triangle* startTri, glm::vec2& p1, glm::vec2& p2, bool boundary=false) {
		this->p1 = p1;
		this->p2 = p2;
		this->boundary = boundary;

		triangle = startTri;
	}

	bool sameEdge(Edge* e) {
		if (e == nullptr) return false;

		return sameEdge(e->p1, e->p2); //(e->p2 - e->p1 == p1 - p2) || (e->p2 - e->p1 == p2 - p1);
	}

	bool sameEdge(glm::vec2 ep1, glm::vec2 ep2) {
		return (ep2 - ep1 == p1 - p2) || (ep2 - ep1 == p2 - p1);
	}

	bool hasPoint(glm::vec2 p) {
		return p == p1 || p == p2;
	}

	void setShared(Edge* shared) {
		this->shared = shared;
	}

	// an edges incident triangle will not change... all we will do when we flip, is we find what edge to change in both triangles

	~Edge() {

	}
};