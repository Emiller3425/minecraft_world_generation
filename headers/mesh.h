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
    std::unordered_set<Block> renderCubes;
    std::unordered_set<glm::vec3> blockPositions;

    Mesh(unordered_set<Chunk> chunks)
    {
        // insert blocks into set
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < size(chunk.blocks); i++)
            {
                if (chunk.blocks[i].blockType != AIR) {
                    blockPositions.insert(chunk.blocks[i].blockPosition);
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR) {
                    blockPositions.insert(chunk.trees.at(i).blockPosition);
                }
            }
            for (int i = 0; i < size(chunk.leaves); i++)
            {
                if (chunk.leaves[i].blockType != AIR) {
                    blockPositions.insert(chunk.leaves[i].blockPosition);
                }
            }
        }

        // Iterate again to determine visible blocks
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < size(chunk.blocks); i++)
            {
                if (chunk.blocks[i].blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.blocks[i].blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.insert(chunk.blocks[i]);
                    }
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.trees.at(i).blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.insert(chunk.trees.at(i));
                    }
                }
            }
            for (int i = 0; i < size(chunk.leaves); i++)
            {
                if (chunk.leaves[i].blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.leaves[i].blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.insert(chunk.leaves[i]);
                    }
                }
            }

        }
    }

    void updateMesh(unordered_set<Chunk> chunks) {
        // clear current render cubes
        //renderCubes.clear();
        // insert blocks into set
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < size(chunk.blocks); i++)
            {
                if (chunk.blocks[i].blockType != AIR) {
                    blockPositions.insert(chunk.blocks[i].blockPosition);
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR) {
                    blockPositions.insert(chunk.trees.at(i).blockPosition);
                }
            }
            for (int i = 0; i < size(chunk.leaves); i++)
            {
                if (chunk.leaves[i].blockType != AIR) {
                    blockPositions.insert(chunk.leaves[i].blockPosition);
                }
            }
        }

        // Iterate again to determine visible blocks
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < size(chunk.blocks); i++)
            {
                if (chunk.blocks[i].blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.blocks[i].blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.insert(chunk.blocks[i]);
                    }
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.trees.at(i).blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.insert(chunk.trees.at(i));
                    }
                }
            }
            for (int i = 0; i < size(chunk.leaves); i++)
            {
                if (chunk.leaves[i].blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.leaves[i].blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        blockPositions.find(glm::vec3(pos.x + 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x - 1, pos.y, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y + 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y - 1, pos.z)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z + 1)) == blockPositions.end() ||
                        blockPositions.find(glm::vec3(pos.x, pos.y, pos.z - 1)) == blockPositions.end())
                    {
                        renderCubes.insert(chunk.leaves[i]);
                    }
                }
            }
        }
    }

private:
};

#endif