#ifndef FRUSTRUM_H
#define FRUSTRUM_H

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

#include "plane.h"

using namespace std;

class Frustrum {
public:
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane frontFace;
    Plane rearFace;

    // finish frustrum header definition

    Frustrum() : topFace(glm::vec3(0.0), 0.0f), bottomFace(glm::vec3(0.0), 0.0f), rightFace(glm::vec3(0.0), 0.0f),
                 leftFace(glm::vec3(0.0), 0.0f), frontFace(glm::vec3(0.0), 0.0f), rearFace(glm::vec3(0.0), 0.0f) {}

    Frustrum(Plane top, Plane bottom, Plane right, Plane left, Plane front, Plane rear) {
        topFace = top;
        bottomFace = bottom;

        rightFace = right;
        leftFace = left;

        frontFace = front;
        rearFace = rear;
    }
};


#endif