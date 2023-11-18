#pragma once
#include "glm.hpp"
#include <stdio.h>

class Primitives {
public:
    static bool inCircle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p) {
        glm::mat3 mat{
            {p1.x - p3.x, p1.y - p3.y, (p1.x - p3.x) * (p1.x - p3.x) + (p1.y - p3.y) * (p1.y - p3.y)},
            {p2.x - p3.x, p2.y - p3.y, (p2.x - p3.x) * (p2.x - p3.x) + (p2.y - p3.y) * (p2.y - p3.y)},
            {p.x - p3.x, p.y - p3.y, (p.x - p3.x) * (p.x - p3.x) + (p.y - p3.y) * (p.y - p3.y)}
        };

        float det = glm::determinant(mat);

        // Only return true if the determinant is poitive
        // Other wise there will be no need for a flip to occur.
        return det > 0;
    }

    static int orientation(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) {
        float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);

        if (glm::abs(val) < 1e-6) {
            return 0; 
        }

        return (val > 0) ? -1 : 1;
    }

    static bool pointOnLine(const glm::vec2& p, const glm::vec2& p1, const glm::vec2& p2, float epsilon = 1e-6) {
        // Calculate the direction vector of the line
        glm::vec2 lineDirection = glm::normalize(p2 - p1);

        glm::vec2 pointDirection = p - p1;
        glm::vec2 projection = glm::dot(pointDirection, lineDirection) * lineDirection;
        float distanceSquared = glm::length(pointDirection - projection);

        // Check if the squared distance is within the epsilon squared
        return distanceSquared < (epsilon * epsilon);
    }

    static int pointInTriangle(const glm::vec2& point, std::vector<glm::vec2> vertices, float epsilon = 1e-6f) {
        glm::vec2 v0 = vertices[0];
        glm::vec2 v1 = vertices[1];
        glm::vec2 v2 = vertices[2];

        glm::vec2 e0 = v1 - v0;
        glm::vec2 e1 = v2 - v0;
        glm::vec2 e2 = point - v0;

        float dot00 = glm::dot(e0, e0);
        float dot01 = glm::dot(e0, e1);
        float dot02 = glm::dot(e0, e2);
        float dot11 = glm::dot(e1, e1);
        float dot12 = glm::dot(e1, e2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);

        // Compute barycentric coordinates
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        if (u >= -epsilon && v >= -epsilon && (u + v) <= 1.0f + epsilon) {

            // Check if the point is on an edge within epsilon
            if (u <= epsilon || v <= epsilon || (u + v) >= 1.0f - epsilon) {
                return 2; // On an edge
            }
            return 1; // Inside the triangle
        }

        return 0; // Outside the triangle
    }

};