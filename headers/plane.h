#ifndef PLANE_H
#define PLANE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <unordered_set>

using namespace std;

class Plane {
public:
    glm::vec3 normal;
    float distance;

    Plane() : normal(glm::vec3(0.0f)), distance(0.0f) {}

    Plane(glm::vec3 position, float planeDistance) {
        normal = glm::vec3(position);
        distance = planeDistance;
    }
};


#endif