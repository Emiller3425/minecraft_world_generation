#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "chunk.h"

using namespace std;

class Mesh
{
public:
    std::vector<glm::vec3> cubes;
    std::vector<glm::vec3> renderCubes;

    Mesh(Chunk chunks[], size_t chunkSize)
    {
        for (int i = 0; i < chunkSize; i++)
        {
            for (int ii = 0; ii < size(chunks[i].cubePositions); ii++)
            {
                // Logic for if we are going to render a chunk
                cubes.push_back(chunks[i].cubePositions[ii]);
            }
        }
        for (int i = 0; i < size(cubes); i++)
        {
                if (
                    find(cubes.begin(), cubes.end(), glm::vec3(cubes[i].x + 1, cubes[i].y, cubes[i].z)) == cubes.end() ||
                    find(cubes.begin(), cubes.end(), glm::vec3(cubes[i].x - 1, cubes[i].y, cubes[i].z)) == cubes.end() ||
                    find(cubes.begin(), cubes.end(), glm::vec3(cubes[i].x, cubes[i].y + 1, cubes[i].z)) == cubes.end() ||
                    find(cubes.begin(), cubes.end(), glm::vec3(cubes[i].x, cubes[i].y - 1, cubes[i].z)) == cubes.end() ||
                    find(cubes.begin(), cubes.end(), glm::vec3(cubes[i].x, cubes[i].y, cubes[i].z + 1)) == cubes.end() ||
                    find(cubes.begin(), cubes.end(), glm::vec3(cubes[i].x, cubes[i].y, cubes[i].z - 1)) == cubes.end()
                    )
                {
                    renderCubes.push_back(cubes[i]);
                }
        }
        cout << size(renderCubes);
    }

private:
};

#endif