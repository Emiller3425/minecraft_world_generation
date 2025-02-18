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

#include "block.h"

#include <unordered_set>

using namespace std;

class Mesh
{
public:
    std::vector<Block> renderCubes;
    std::unordered_set<glm::vec3> blockPositions;

    Mesh(Chunk chunks[], size_t chunkSize)
    {
        // Insert all block positions into the set
        for (int i = 0; i < chunkSize; i++)
        {
            for (int ii = 0; ii < size(chunks[i].blocks); ii++)
            {
                if (chunks[i].blocks[ii].blockType != AIR)
                {
                    blockPositions.insert(chunks[i].blocks[ii].blockPosition);
                }
            }
        }

        // Iterate again to determine visible blocks
        for (int i = 0; i < chunkSize; i++)
        {
            for (int ii = 0; ii < size(chunks[i].blocks); ii++)
            {
                if (chunks[i].blocks[ii].blockType != AIR)
                {
                    const glm::vec3 &pos = chunks[i].blocks[ii].blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.push_back(chunks[i].blocks[ii]);
                    }
                }
            }
        }
    }

private:
};

#endif