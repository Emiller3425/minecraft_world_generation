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
    std::unordered_set<Block> renderOpaqueCubes;
    std::unordered_set<Block> renderTransparentCubes;
    std::unordered_map<glm::vec3, int> blockPositions;


    Mesh(unordered_set<Chunk> chunks)
    {
        // insert blocks into set
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < chunk.blocks.size(); i++)
            {
                if (chunk.blocks.at(i).blockType != AIR) {
                    blockPositions[chunk.blocks.at(i).blockPosition] = chunk.blocks.at(i).blockType;
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR) {
                    blockPositions[chunk.trees.at(i).blockPosition] = chunk.trees.at(i).blockType;
                }
            }
            for (int i = 0; i < chunk.leaves.size(); i++)
            {
                if (chunk.leaves.at(i).blockType != AIR) {
                    blockPositions[chunk.leaves.at(i).blockPosition] = chunk.leaves.at(i).blockType;
                }
            }
        }

         auto isMissingOrTransparent = [&](const glm::vec3& checkPos) {
            auto it = blockPositions.find(checkPos);
            if (it == blockPositions.end()) {
                return true;
            }
            return it->second == 5;
        };

        // Iterate again to determine visible blocks
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < chunk.blocks.size(); i++)
            {
                if (chunk.blocks.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.blocks.at(i).blockPosition;

                    if (
                        isMissingOrTransparent(glm::vec3(pos.x + 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x - 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y + 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y - 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z + 1)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z - 1))
                    ) 
                    {
                        renderOpaqueCubes.insert(chunk.blocks.at(i));
                    }
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.trees.at(i).blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if
                    (
                        isMissingOrTransparent(glm::vec3(pos.x + 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x - 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y + 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y - 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z + 1)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z - 1))
                    ) 
                    {
                        renderOpaqueCubes.insert(chunk.trees.at(i));
                    }
                }
            }
            for (int i = 0; i < chunk.leaves.size(); i++)
            {
                if (chunk.leaves.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.leaves.at(i).blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        isMissingOrTransparent(glm::vec3(pos.x + 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x - 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y + 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y - 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z + 1)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z - 1))
                    ) 
                    {
                        renderTransparentCubes.insert(chunk.leaves.at(i));
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
            for (int i = 0; i < chunk.blocks.size(); i++)
            {
                if (chunk.blocks.at(i).blockType != AIR) {
                    blockPositions[chunk.blocks.at(i).blockPosition] = chunk.blocks.at(i).blockType;
                }
            }
            for (int i = 0; i < chunk.trees.size(); i++)
            {
                if (chunk.trees.at(i).blockType != AIR) {
                    blockPositions[chunk.trees.at(i).blockPosition] = chunk.trees.at(i).blockType;
                }
            }
            for (int i = 0; i < chunk.leaves.size(); i++)
            {
                if (chunk.leaves[i].blockType != AIR) {
                    blockPositions[chunk.leaves.at(i).blockPosition] = chunk.leaves.at(i).blockType;
                }
            }
        }

        auto isMissingOrTransparent = [&](const glm::vec3& checkPos) {
            auto it = blockPositions.find(checkPos);
            if (it == blockPositions.end()) {
                return true;
            }
            return it->second == 5;
        };

        // Iterate again to determine visible blocks
        for (const auto &chunk : chunks)
        {
            for (int i = 0; i < chunk.blocks.size(); i++)
            {
                if (chunk.blocks.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.blocks.at(i).blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        isMissingOrTransparent(glm::vec3(pos.x + 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x - 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y + 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y - 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z + 1)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z - 1))
                    ) 
                    {
                        renderOpaqueCubes.insert(chunk.blocks.at(i));
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
                        isMissingOrTransparent(glm::vec3(pos.x + 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x - 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y + 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y - 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z + 1)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z - 1))
                    ) 
                    {
                        renderOpaqueCubes.insert(chunk.trees.at(i));
                    }
                }
            }
            for (int i = 0; i < chunk.leaves.size(); i++)
            {
                if (chunk.leaves.at(i).blockType != AIR)
                {
                    const glm::vec3 &pos = chunk.leaves.at(i).blockPosition;

                    // Check if block is exposed (i.e., it has at least one open face)
                    if (
                        isMissingOrTransparent(glm::vec3(pos.x + 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x - 1, pos.y, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y + 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y - 1, pos.z)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z + 1)) ||
                        isMissingOrTransparent(glm::vec3(pos.x, pos.y, pos.z - 1))
                    ) 
                    {
                        renderTransparentCubes.insert(chunk.leaves.at(i));
                    }
                }
            }
        }
    }

private:
};

#endif