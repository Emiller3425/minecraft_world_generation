#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Chunk {

    // TODO: add terrain generation using perlin noise and linear interpolation

    public:
    const unsigned int CHUNK_SIZE = 16;
    glm::vec3 cubePositions [4096];
    glm::vec3 origin;

    Chunk(glm::vec3 originVector) {
        origin = originVector;
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int ii = 0; ii < CHUNK_SIZE; ii++) {
                for (int iii = 0; iii < CHUNK_SIZE; iii++) {
                    cubePositions[(i * (CHUNK_SIZE * CHUNK_SIZE)) + (ii + (iii * CHUNK_SIZE))] = glm::vec3(static_cast<float>(i + origin.x), static_cast<float>(ii + origin.y), static_cast<float>(iii + origin.z));
                }
            }
        }
    }

    glm::vec3 GetOrigin() {
        return origin;
    }
    private:
    // IDK what to put here yet
};

#endif