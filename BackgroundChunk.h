#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "DAG.h"
#include "PoissonSampler.h"
#include <iostream>
#include "Triangle.h"
#include "Edge.h"
#include "GLFW/glfw3.h"
#include "World.h"
#include <chrono>

class BackgroundChunk {
public:
	int drawSize = 0;
	int sx = 0, sz = 0;
	GLuint vao;
	GLuint vID;


	BackgroundChunk(int sx, int sz, float testDist=0) {
		this->sx = sx;
		this->sz = sz;

	
		std::vector<glm::vec2> points;
		
		if (testDist != 0) {
			points = PoissonSampler::generatePoints(0, 0, true, testDist);
		} else{
			points = PoissonSampler::generatePoints(sx, sz);
		}

		auto start = std::chrono::high_resolution_clock::now();

		glm::vec2 p1(0, 0);
		glm::vec2 p2(LOD_SIZE * 2, 0);
		glm::vec2 p3(0, LOD_SIZE * 2);
		Triangle* root = new Triangle(p1, p2, p3, true);

		DAG* dag = new DAG(root); // this has the root triangle 
		// the root triangle needs to be an arbitrarily big triangle that can full cover a LOD_SIZExLOD_SIZE area

		for (int i = 0; i < points.size(); i++) {
			glm::vec2 p = points[i];

			// look for the triangle
			Triangle* cur = dag->findTriangle(root, p);

			if (cur == nullptr) continue;

			Edge* incidentEdge = nullptr;

			for (int j = 0; j < 3; j++) {
				Edge* e = cur->edges[j];
				if (Primitives::pointOnLine(p, e->p1, e->p2)) {
					// we know this edge is the one we want to look at.
					// swegg
					incidentEdge = e;
					break;
				}
			}


			// 3 point case
			if (incidentEdge == nullptr) {
				//divide the current triangle into 3 other triangles

				Triangle* t1 = new Triangle(cur->edges[0]->p1, cur->edges[1]->p1, p);
				Triangle* t2 = new Triangle(cur->edges[1]->p1, cur->edges[2]->p1, p);
				Triangle* t3 = new Triangle(cur->edges[2]->p1, cur->edges[0]->p1, p);

				Triangle* tris[3] = { t1, t2, t3 };

				/*

				for each sub triangle

					find edge that is incident on the parent triangle
					meaning the edge was found in each triangle

					so we have edge and parent edge

					if parentEdge->shared is not null
						set edge->shared->shared (shared edge)
						set edge->shared =


				*/


				/*
				* 
				Here we are matching up the edges of the parent
				Triangle to the newly created child triangles
				Triangles should NEVER share an edge with triangle that has children

				Additionally, we must legalize the edges of the original triangle
				That are contained in the new triangles (1 edge per tri)

				*/

				std::vector<Edge*> toLegalize;

				for (int j = 0; j < 3; j++) {
					Edge* edge = cur->edges[j];

					for (int k = 0; k < 3; k++) {
						Edge* e1 = tris[k]->findEdge(edge);

						if (e1 != nullptr && edge->shared != nullptr && !edge->boundary) {
							
							// THESE DEBUG LINES FIXED THE ENTIRE PROJECT
							// :)
							// 
							//std::cout << "e1 has " << e1->triangle->children.size() << "children" << std::endl;
							//std::cout << "edge->shared has " << edge->shared->triangle->children.size() << "children" << std::endl;


							e1->shared = edge->shared;
							edge->shared->shared = e1;

							toLegalize.push_back(e1);

						}
					}
					// this is bad code but it ok :)
				}


				fixShared(t1, t2);
				fixShared(t1, t3);
				fixShared(t3, t2);

				cur->edges[0]->shared = nullptr;
				cur->edges[1]->shared = nullptr;
				cur->edges[2]->shared = nullptr;

				cur->children.push_back(t1);
				cur->children.push_back(t2);
				cur->children.push_back(t3);


				for (int i = 0; i < toLegalize.size() && i < 3; i++) {
					legalizeEdge(toLegalize[i], p);
				}

			}
			else {
				// The 4 point case is not implemented yet
				// The likelihood of this happening, especially with the poisson distribution
				// is not worth the time commitment right now (finals are overwhelming)
			}
		}

		std::vector<Triangle*> tris = dag->extract();

		if (testDist != 0) {
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;

			std::cout << "n: " << points.size() << "| time: " << duration.count() << std::endl;
		}

		GLfloat* verts = (GLfloat*)malloc(sizeof(GLfloat) * 3 * 3 * tris.size());


		int index = 0;
		for (int i = 0; i < tris.size(); i++) {
			Triangle* t = tris[i];
			auto v = t->getVertices();

			for (int j = 0; j < 3; j++) {
				auto p = v[j];
				verts[index++] = p.x;
				verts[index++] = (World::getHeight(sx + p.x, sz + p.y));
				verts[index++] = (p.y);
			}
		}

		drawSize = 3 * 3 * tris.size();

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vID); // bind vertices

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vID);

		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 3 * tris.size(), verts, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0); // Unbind VAO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);


		// graphics section now 
	}

	void fixShared(Triangle* t1, Triangle* t2) {
		// match edges between two triangles

		for (int j = 0; j < 3; j++) {
			Edge* e1 = t1->edges[j];

			Edge* e2 = t2->findEdge(e1);

			if (e2 != nullptr) {
				e2->shared = e1;
				e1->shared = e2;
			}
		}
	}

	void render(GLuint modelViewID) {
		glm::mat4 modelView = glm::translate(glm::mat4(1), glm::vec3(sx, 0, sz));

		glUniformMatrix4fv(modelViewID, 1, GL_FALSE, glm::value_ptr(modelView));

		//glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vID); // bind the vertices
		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, drawSize);
		glBindBuffer(GL_ARRAY_BUFFER, 0); // bind the vertices
		glBindVertexArray(0);
	}


	void legalizeEdge(Edge* e, glm::vec2 q) {

		if (!e->boundary) {
			Triangle* cur = e->triangle;
			Triangle* adj = e->shared->triangle;

			if (cur == adj) {
				std::cout << "This should never happen. -BackgroundChunk.h in legalizeEdge()" << std::endl;
			}


			glm::vec2 p = adj->findNotOnEdge(e);
			glm::vec2 p1 = e->p1;
			glm::vec2 p2 = e->p2;

			/*
				p
			    /\
			   /  \
			  /    \
			 /      \
			/        \
		p1 ------------ p2
			\        /
			 \      /
			  \    /
			   \  /
			    \/
			     q
			
			*/

			if (Primitives::orientation(p, p1, p2) == -1) {
				// Originally we thought this was necessary but it breaks things
				// The following code accounts for any orientation
				// Because we search for the edges based off of these glm::vec2s
			}

			if (Primitives::inCircle(p, p1, p2, q)) {
				// sanity check
				if (cur->children.size() == 0 && adj->children.size() == 0) {

					// Make a copy of our triangles
					// Since we want to preserve the old one's for DAG Searching
					// Triangles (p1 p2 q), (p1, q, p2) will both have children (p1 q p) and (q p2 p)

					Triangle* adjCopy = new Triangle(*adj);
					Triangle* curCopy = new Triangle(*cur);

					Triangle* qParent = nullptr;
					Triangle* pParent = nullptr;

					Edge* p1q = adjCopy->findEdge(p1, q);


					//Find the right edges (and right triangles)

					Edge* p1p = nullptr;
					if (p1q == nullptr) {

						p1q = curCopy->findEdge(p1, q);
						p1p = adjCopy->findEdge(p1, p);

						qParent = cur;
						pParent = adj;
					}
					else {
						p1p = curCopy->findEdge(p1, p);

						qParent = adj;
						pParent = cur;
					}


					// Flip the vertices

					Triangle* qtri = p1q->triangle;
					Triangle* ptri = p1p->triangle;


					qtri->changeVertexTo(p2, p, pParent);
					ptri->changeVertexTo(p1, q, qParent);



					Edge* qe = qtri->findEdge(p, q);
					Edge* pe = ptri->findEdge(p, q);

					// Fix the shared edges

					if (qe != nullptr) {

						qe->shared = pe;
					}

					if (pe != nullptr) {
						pe->shared = qe;
					}

					//Push the right children

					adj->children.push_back(qtri);
					adj->children.push_back(ptri);

					// Might not be necessary but whatever
					adj->edges[0]->shared = nullptr;
					adj->edges[1]->shared = nullptr;
					adj->edges[2]->shared = nullptr;

					cur->children.push_back(qtri);
					cur->children.push_back(ptri);

					cur->edges[0]->shared = nullptr;
					cur->edges[1]->shared = nullptr;
					cur->edges[2]->shared = nullptr;

					// Recursive legalizeEdge step.

					auto leg1 = ptri->findEdge(p2, p);
					auto leg2 = qtri->findEdge(p1, p);

					if (leg1 != nullptr && leg1->shared != nullptr) {
						legalizeEdge(leg1, q);
					}

					if (leg1 != nullptr && leg2->shared != nullptr) {
						legalizeEdge(leg2, q);
					}
				}
				else {
					std::cout << "Something is wrong in legalizeEdge(). We are edge sharing with a non-leaf triangle" << std::endl;
				}
			}

		}
	}

	~BackgroundChunk();
private:
	;

};