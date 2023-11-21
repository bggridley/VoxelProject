/*

Author Stephen Derenski and Ben Gridley

*/

#pragma once
#include <glm/glm.hpp>
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
        // Could also just check orientation between the two points lol
        // this is only used once and 
        // gets the right answers

        glm::vec2 lineDirection = glm::normalize(p2 - p1);

        glm::vec2 pointDirection = p - p1;
        glm::vec2 projection = glm::dot(pointDirection, lineDirection) * lineDirection;
        float distanceSquared = glm::length(pointDirection - projection);

        return distanceSquared < (epsilon * epsilon);
    }

    static int pointInTriangle(const glm::vec2& point, std::vector<glm::vec2> v, float epsilon = 1e-6f) {
        // Barycentric coords don't seem to work too well and seems like way more work
        // orientation good :)

        return orientation(v[0], v[1], point) >= 0 && orientation(v[1], v[2], point) >= 0 && orientation(v[2], v[0], point) >= 0;
    }

};