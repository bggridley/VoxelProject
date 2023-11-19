#pragma once
#include <vector>
#include "glm.hpp"
#include "DAG.h"
#include "PoissonSampler.h"
#include <iostream>
#include "Triangle.h"
#include "Edge.h"
#include "GLFW/glfw3.h"
#include "World.h"
#include <chrono>
// not worry about any of the implementation details right now for this
// all we want to do is just generate the delaunay triangulation based on the Poisson Points
// poisson sampling is 256x256


class BackgroundChunk {
public:
	int drawSize = 0;
	int sx = 0, sz = 0;
	GLuint vao;
	GLuint vID;


	BackgroundChunk(int sx, int sz, float testDist=0) {
		this->sx = sx;
		this->sz = sz;

		//std::cout << "INIT background" << std::endl;
		std::vector<glm::vec2> points;
		
		if (testDist != 0) {
			//std::cout << "gettin the test points" << std::endl;
			points = PoissonSampler::generatePoints(0, 0, true, testDist);
		} else{
			points = PoissonSampler::generatePoints(sx, sz);
		}
			 // none of these args are actually used. fill in later with LOD_SIZZe  offset and what not bla hblah blah


		auto start = std::chrono::high_resolution_clock::now();

		glm::vec2 p1(0, 0);
		glm::vec2 p2(LOD_SIZE * 2, 0);
		glm::vec2 p3(0, LOD_SIZE * 2);
		Triangle* root = new Triangle(p1, p2, p3, true);

		DAG* dag = new DAG(root); // this has the root triangle // the root triangle needs to be an arbitrarily big triangle that can full cover a 256x256 area

		for (int i = 0; i < points.size(); i++) {
			//std::cout << "actually got here" << std::endl;
			glm::vec2 p = points[i];

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


			if (incidentEdge == nullptr) {
				// 3 edges to check
				//std::cout << "not on an edge" << std::endl;

				//find point that is NOT on the triangle



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

				std::vector<Edge*> toLegalize;

				for (int j = 0; j < 3; j++) {
					Edge* edge = cur->edges[j];

					for (int k = 0; k < 3; k++) {
						Edge* e1 = tris[k]->findEdge(edge);

						if (e1 != nullptr && edge->shared != nullptr && !edge->boundary) {
							
							// THESE DEBUG LINES FIXED THE ENTIRE PROJECT
							// :)
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
				//std::cout << "ruh roh, an edge!" << std::endl;
			}
		}

		std::vector<Triangle*> tris = dag->extract();

		if (testDist != 0) {
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;

			std::cout << "n: " << points.size() << "| time: " << duration.count() << std::endl;
		}

		GLfloat* verts = (GLfloat*)malloc(sizeof(GLfloat) * 3 * 3 * tris.size());
		//std::cout << "final triangulation size is" << tris.size() << std::endl;



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

		// IF WE FLIP AN EDGE< we HAVE TO UPDATE TEH DAG!!!!
		// OR WE JUST SET A FLAG WITHIN A TRIANGLE THAT STATES IT IS NO LONGER GEOMETRICALLY APPLICABLE
		// AND IS MERELY JUST FOR THE SEARCH STRUCTURE????
		// not really sure .. . .. . . .
		// gonna kms

		if (!e->boundary) {
			Triangle* cur = e->triangle;
			Triangle* adj = e->shared->triangle;

			if (cur == adj) {
				std::cout << "HOW THE HELL" << std::endl;
			}



			glm::vec2 p = adj->findNotOnEdge(e);
			glm::vec2 p1 = e->p1;
			glm::vec2 p2 = e->p2;

			if (Primitives::orientation(p, p1, p2) == -1) {
	
			}

			if (Primitives::inCircle(p, p1, p2, q)) {
				if (cur->children.size() == 0 && adj->children.size() == 0) {
					//std::cout << "found that to be illegal " << std::endl << std::endl;


					//std::cout << "we found an illegal edge" << p1.x << ", " << p1.y << " | " << p2.x << ", " << p2.y << std::endl;


					//std::cout << "PRINTIN OUR TEST TRIANGLES" << std::endl;

					//cur->print();
					//adj->print();
					//std::cout << q.x << ", " << q.y << std::endl; 
					//

					//std::cout << "AAAAAAAAAAAA" << std::endl;



					 // we know this one for certain i think
					//adj->setEdges(q, p2, p);
					//cur->setEdges(p1, q, p);

					Triangle* adjCopy = new Triangle(*adj);
					Triangle* curCopy = new Triangle(*cur);

					Triangle* qParent = nullptr;
					Triangle* pParent = nullptr;

					Edge* p1q = adjCopy->findEdge(p1, q);


					//adjCopy->print();
					//adj->print();
					//std::cout << "done";

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



					Triangle* qtri = p1q->triangle;
					Triangle* ptri = p1p->triangle;


					/*				std::cout << "before" << std::endl;


									ptri->print();

									qtri->print();*/

					qtri->changeVertexTo(p2, p, pParent);
					ptri->changeVertexTo(p1, q, qParent);



					Edge* qe = qtri->findEdge(p, q);
					Edge* pe = ptri->findEdge(p, q);

					if (qe != nullptr) {

						qe->shared = pe;
					}

					if (pe != nullptr) {
						pe->shared = qe;
					}

					/*

								ptri->print();
								qtri->print();


								std::cout << "after" << std::endl;*/



								// on q tri we want p1 p
								// on ptri we want 

								//cur->children.

								//std::cout << "pushing the babies" << std::endl;

					adj->children.push_back(qtri);
					adj->children.push_back(ptri);

					adj->edges[0]->shared = nullptr;
					adj->edges[1]->shared = nullptr;
					adj->edges[2]->shared = nullptr;

					cur->children.push_back(qtri);
					cur->children.push_back(ptri);

					cur->edges[0]->shared = nullptr;
					cur->edges[1]->shared = nullptr;
					cur->edges[2]->shared = nullptr;

					// now we just need to find the triangle sthat containas p1p and p2p after the shift
					// and legalize them

					auto leg1 = ptri->findEdge(p2, p);
					auto leg2 = qtri->findEdge(p1, p);

					if (leg1 != nullptr && leg1->shared != nullptr) {
						legalizeEdge(leg1, q);
					}//

					if (leg1 != nullptr && leg2->shared != nullptr) {
						legalizeEdge(leg2, q);
					}


					// now we want the other two edges on 

				}
				else {
					std::cout << "something wrong" << std::endl;
				}
			}

		}
	}


	void byProjection() {
		std::vector<GLfloat> triPoints = std::vector<GLfloat>();
		std::vector<glm::vec3> p = std::vector<glm::vec3>();

		std::vector<glm::vec2> pts = PoissonSampler::generatePoints(0, 0); // none of these args are actually used. fill in later with LOD_SIZZe  offset and what not bla hblah blah


		for (int i = 0; i < pts.size(); i++) {
			float x = pts[i].x;
			float y = pts[i].y;

			p.push_back(glm::vec3(x, x * x + y * y, y));
		}

		int i, j, k, m, numTrian = 0;
		float xn, yn, zn;
		int flag = 0;

		int n = p.size();
		for (i = 0; i < n; i++) {
			for (j = i + 1; j < n; j++) {
				for (k = i + 1; k < n; k++) {
					if (j != k) {
						xn = (p[j].y - p[i].y) * (p[k].z - p[i].z) - (p[k].y - p[i].y) * (p[j].z - p[i].z);
						yn = (p[k].x - p[i].x) * (p[j].z - p[i].z) - (p[j].x - p[i].x) * (p[k].z - p[i].z);
						zn = (p[j].x - p[i].x) * (p[k].y - p[i].y) - (p[k].x - p[i].x) * (p[j].y - p[i].y);

						if (flag = (zn < 0)) {
							for (m = 0; m < n; m++) {
								flag = flag & (((p[m].x - p[i].x) * xn +
									(p[m].y - p[i].y) * yn + (p[m].z - p[i].z) * zn) <= 0);
							}
						}

						if (flag) {
							// check orientation here :)

							if (Primitives::orientation(p[i], p[j], p[k]) == 1) {
								pushPoint(triPoints, p[i]);
								pushPoint(triPoints, p[j]);
								pushPoint(triPoints, p[k]);
								std::cout << "pushin a point" << std::endl;
							}
							else {
								pushPoint(triPoints, p[k]);
								pushPoint(triPoints, p[j]);
								pushPoint(triPoints, p[i]);
								std::cout << "pushin a point" << std::endl;

							}
						}
					}
				}
			}
		}

		drawSize = triPoints.size();


		std::cout << triPoints.size() << std::endl;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vID); // bind vertices

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vID);

		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * triPoints.size(), &triPoints[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0); // Unbind VAO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		std::cout << " got here" << std::endl;





	}

	void pushPoint(std::vector<GLfloat> vec, glm::vec3 p) {
		vec.push_back(p.x);
		vec.push_back(World::getHeight(p.x, p.z));
		vec.push_back(p.z);
	}

	~BackgroundChunk();
private:
	;

};