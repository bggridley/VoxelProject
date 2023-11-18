#pragma once
#include <vector>
#include "glm.hpp"
#include "DAG.h"
#include "PoissonSampler.h"
#include <iostream>
#include "Triangle.h"
#include "Edge.h"
// not worry about any of the implementation details right now for this
// all we want to do is just generate the delaunay triangulation based on the Poisson Points
// poisson sampling is 256x256
constexpr int LOD_SIZE = 256;

class BackgroundChunk {
public:
	BackgroundChunk() {

	}

	void init() {
		std::cout << "INIT background" << std::endl;
		std::vector<glm::vec2> points = std::vector<glm::vec2>();//PoissonSampler::generatePoints(0, 0, 0, 0); // none of these args are actually used. fill in later with LOD_SIZZe  offset and what not bla hblah blah
		glm::vec2 p1(0, 0);
		glm::vec2 p2(LOD_SIZE * 2, 0);
		glm::vec2 p3(0, LOD_SIZE * 2);
		Triangle* root = new Triangle(p1, p2, p3, true);

		points.push_back(glm::vec2(128, 128));
		points.push_back(glm::vec2(50, 48));


		DAG* dag = new DAG(root); // this has the root triangle // the root triangle needs to be an arbitrarily big triangle that can full cover a 256x256 area

		for (int i = 0; i < points.size(); i++) {
			std::cout << "actually got here" << std::endl;
			glm::vec2 p = points[i];

			// find the triangle that the point is in

			Triangle* cur = dag->findTriangle(root, p);
			// add the new sub triangls, ensuring that the edges are the same

			std::cout << (cur == nullptr) << std::endl;
			// check if it's incident on an edge or not !!!

			Edge* incidentEdge = nullptr;

			for (int i = 0; i < 3; i++) {
				Edge* e = cur->edges[i];
				if (Primitives::pointOnLine(p, e->p1, e->p2)) {
					// we know this edge is the one we want to look at.
					// swegg
					incidentEdge = e;
					break;
				}
			}


			if (incidentEdge == nullptr) {
				// 3 edges to check
				std::cout << "not on an edge" << std::endl;

				//find point that is NOT on the triangle
				Triangle* t1 = new Triangle(cur->edges[0]->p1, cur->edges[1]->p1, p);
				Triangle* t2 = new Triangle(cur->edges[1]->p1, cur->edges[2]->p1, p);
				Triangle* t3 = new Triangle(cur->edges[2]->p1, cur->edges[0]->p1, p);

				// for more accurate descriptno of this see dessktop png. triangle3.png
				t1->edges[2]->shared = t3->edges[1];
				t1->edges[1]->shared = t2->edges[2];
				t1->edges[0]->shared = cur->edges[0]->shared;

			
				t2->edges[2]->shared = t1->edges[1];
				t2->edges[1]->shared = t3->edges[2];
				t2->edges[0]->shared = cur->edges[1]->shared;
				
	
				t3->edges[2]->shared = t2->edges[1];
				t3->edges[1]->shared = t1->edges[2];
				t3->edges[0]->shared = cur->edges[2]->shared;
				

				if (t1->edges[0]->shared != nullptr) {
					t1->edges[0]->shared->shared = t1->edges[0];
				}

				if (t2->edges[0]->shared != nullptr) {
					t2->edges[0]->shared->shared = t2->edges[0];
				}

				if (t3->edges[0]->shared != nullptr) {
					t3->edges[0]->shared->shared = t3->edges[0];
				}

				/// ^^^ WE CAN REPLACE THIS BS LATER WITH A NICE "setSharedBetween" in triangle class
				// can literally just do some clever vector math. since the edges are guaranteed to be in
				// CCW order all the time, we can literally just do a 3 x 3 check on vec1 - vec2. if they're
				// the same, then we set the shared edges between them  :) 

				// now, for each edge, we just have to find the
				// point on the triangle adjacent through shared

				// get the OUTER one's from the PARENT
				// this is actually really cool

				// we know each case uses the edge from CUR, and some other point on an adjacent triangle
				
				// the adjacent triangle
				// -> shared is the edge we will flip if it happens

				cur->children.push_back(t1);
				cur->children.push_back(t2);
				cur->children.push_back(t3);
				// we also need to consider what happens if and edge flip appened incident on
				// the current triangle
				
				// add the kiddos

				Edge* shared3 = t1->edges[0];
				Edge* shared2 = t2->edges[0];
				Edge* shared1 = t3->edges[0];

				if (shared3 != nullptr && shared3->shared != nullptr) {
					legalizeEdge(shared3, p);
				}

				if (shared2 != nullptr && shared2->shared != nullptr) {
					legalizeEdge(shared2, p); 
				}

				if (shared1 != nullptr && shared1->shared != nullptr) {
					legalizeEdge(shared1, p); 
				}









				//glm::vec2 testvec = (t1->edges[1]->p2 - t2->edges[2]->p1);

				//std::cout << "SHOULD BE 0 " << testvec.x << ", " << testvec.y << std::endl;




			}
			else {
				std::cout << "yes, an edge!" << std::endl;
				// we can actually find a way to completely avoid doing this.... we can just do a check and if it's 
				// on a point, we'll shift it some direction
				// that'll at least save us some time


				// 4 edges to check

			}
		}

		dag->print();
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

			glm::vec2 p = adj->findNotOnEdge(e);
			glm::vec2 p1 = e->p1;
			glm::vec2 p2 = e->p2;

			if (Primitives::orientation(p, p1, p2) == 1) {
				p2 = p;
				p = e->p2;

				std::cout << "orientation broke" << std::endl;
			}

			//cur->print();
			//adj->print();


			// okay so this is right ^^^
			// up here
			// it's jsut the bit where we find out the other points that is bad !!



			std::cout << "checking in circle with points" << std::endl;

			std::cout << p.x << ", " << p.y << std::endl;
			std::cout << p1.x << ", " << p1.y << std::endl;
			std::cout << p2.x << ", " << p2.y << std::endl;
			std::cout << q.x << ", " << q.y << std::endl;


			if (Primitives::inCircle(p, p1, p2,	q)) {
				std::cout << "found that to be illegal " << std::endl << std::endl;


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

				qtri->changeVertexTo(p2, p, pParent);
				ptri->changeVertexTo(p1, q, qParent);

				





				if (cur->children.size() == 0 && adj->children.size() == 0) {
					//cur->children.

					std::cout << "pushing the babies" << std::endl;

					adj->children.push_back(qtri);
					adj->children.push_back(ptri);

					cur->children.push_back(qtri);
					cur->children.push_back(ptri);
				}
				else {
					std::cout << "something wrong" << std::endl;
				}
				
				


			}

		}
	}

	~BackgroundChunk();
private:
	;

};