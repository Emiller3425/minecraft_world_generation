#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <math.h>
#include <random>
#include <functional> // For std::hash
#include <unordered_set>

#include "block.h"
#include "chunk.h"

using namespace std;

const int AIR = 0;
const int GRASS = 1;
const int DIRT = 2;
const int SAND = 3;
const int TREE = 4;
const int LEAF = 5;

float perlin(float x, float y);
float dotGridGradient(int ix, int iy, float x, float y);
float interpolate(float a0, float a1, float w);
glm::vec2 randomGradient(int ix, int iy);

float temp;
float min_val;
float max_val;
std::random_device rd; 
std::mt19937 gen(rd());

// TODO seeds and fix perlin noise being outside of [-1, 1]

using namespace std;

class Chunk
{

// TODO instead of a vector of blocks, do a vector of positions, create one block type of each, and draw at all locations

public:
    // chunk size
    static const unsigned int CHUNK_SIZE = 16;
    // array of blocks in chunk
    std::vector<Block> blocks;
    
    std::unordered_map<glm::vec3, int> blocks_;

    // vector for list of tree trunks
    std::vector<Block> trees;

    std::unordered_map<glm::vec3, int> trees_;

    // vector for list of leaves
    std::vector<Block> leaves;

    std::unordered_map<glm::vec3, int> leavs_;

    // coordinate origin for this chunk
    glm::vec3 origin;

    Chunk(glm::vec3 originVector)
    {
        // Vector for origin coordinates
        origin = originVector;
        // scaler for noise
        float noiseScaler = 0.05f;
        // decide max terrain height
        float maxTerrainHeight = (float)CHUNK_SIZE - 1;
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                // Sample perlin for height at x,z
                float rawNoise = perlin((x + origin.x) * noiseScaler, (z + origin.z) * noiseScaler);

                // noise returns [-1, 1] normalize to [0, 1]
                float normalizedNoise = (rawNoise + 1.0f) / 2.0f;

                // Scale to [0, maxTerrainHeight]
                float terrainHeightF = normalizedNoise * maxTerrainHeight;

                // Turn into an int for the cutoff
                int terrainHeight = (int)floor(terrainHeightF);

                for (int y = 0; y < CHUNK_SIZE; y++)
                {
                    int index = (x * CHUNK_SIZE * CHUNK_SIZE) + (y + (z * CHUNK_SIZE));

                    // set block type
                    if (y == terrainHeight && y > 3)
                    {
                    blocks.push_back(Block(glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z)), GRASS));
                    blocks_[glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z))] = GRASS;
                    }
                    else if (y < terrainHeight && y > 3)
                    {
                    blocks.push_back(Block(glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z)), DIRT));
                    blocks_[glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z))] = DIRT;
                    }
                    else if (y <= 3)
                    {
                    blocks.push_back(Block(glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z)), SAND));
                    blocks_[glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z))] = SAND;
                    }
                    else
                    {
                    blocks.push_back(Block(glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z)), AIR));
                    blocks_[glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + origin.y), static_cast<float>(z + origin.z))] = AIR;
                    }

                    // TODO random trees, random heights, leaves on trees

                    // make a vector of trees instead of an array, this is per chunk, trees can be pushed instead of using an array
                    if (blocks.at(index).blockType == GRASS && trees.size() == 0) {
                        // loop to add tree blocks
                        for (int i = 0; i < 6; i++)
                        {
                            trees.push_back(Block(glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + (i + 1) + origin.y), static_cast<float>(z + origin.z)), TREE));
                            trees_[glm::vec3(static_cast<float>(x + origin.x), static_cast<float>(y + (i + 1) + origin.y), static_cast<float>(z + origin.z))] = TREE;
                        }
                        // loop to add leaf blocks, i feel like it's gotta be a vector so we can push_back
                        for (int i = 0; i < 4; i ++) {
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 1) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 1) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z + 1)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z - 1)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 1) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z + 1)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 1) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z - 1)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 1) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z + 1)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 1) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z - 1)), LEAF));
                            // side leaves
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 2) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 2) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z + 2)), LEAF));
                            leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + (i + 1) + origin.y + 2), static_cast<float>(z + origin.z - 2)), LEAF));
                        }
                        // loop to add leaf blocks, i feel like it's gotta be a vector so we can push_back
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 1) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z + 1)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 1) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z - 1)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 1) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z + 1)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 1) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z - 1)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x + 1) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z - 1)), LEAF));
                        leaves.push_back(Block(glm::vec3(static_cast<float>(x + origin.x - 1) , static_cast<float>(y + origin.y + 7), static_cast<float>(z + origin.z + 1)), LEAF));
                    }
                }
            }
        }
    }

    void generateTrees()
    {
    }

    glm::vec2 randomGradient(int ix, int iy)
    {
        const unsigned w = 8 * sizeof(unsigned);
        const unsigned s = w / 2;
        unsigned a = ix, b = iy;
        a *= 3284157443;

        b ^= a << s | (a > w - s);
        b *= 1911520717;

        a ^= b << s | (b >> (w - s));
        b *= 2048419325;
        float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*pi]

        glm::vec2 v;

        v.x = sin(random);
        v.y = cos(random);

        return v;
    }

    // Computes the dot product of the distance of gradient vecotrs.
    float dotGridGradient(int ix, int iy, float x, float y)
    {
        glm::vec2 gradient = randomGradient(ix, iy);

        // Compute the distance vector
        float dx = x - (float)ix;
        float dy = y - (float)iy;

        // Compute the dot-product
        return (dx * gradient.x + dy * gradient.y);
    }

    float interpolate(float a0, float a1, float w)
    {
        return (a1 - a0) * (3.0f - w * 2.0f) * w * w + a0;
    }

    // Sample Perlin noise at coordinates x, y
    float perlin(float x, float y)
    {

        // Determine grid cell corner coordinates
        int x0 = (int)x;
        int y0 = (int)y;
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        // Compute Interpolation weights
        float sx = x - (float)x0;
        float sy = y - (float)y0;

        // Compute and interpolate top two corners
        float n0 = dotGridGradient(x0, y0, x, y);
        float n1 = dotGridGradient(x1, y0, x, y);
        float ix0 = interpolate(n0, n1, sx);

        // Compute and interpolate bottom two corners
        n0 = dotGridGradient(x0, y1, x, y);
        n1 = dotGridGradient(x1, y1, x, y);
        float ix1 = interpolate(n0, n1, sx);

        // Final step: interpolate between the two perviously interpolated values, now in y
        float value = interpolate(ix0, ix1, sy);

        return glm::clamp(value, -1.0f, 1.0f);
    }

    glm::vec3 GetOrigin()
    {
        return origin;
    }

    bool operator==(const Chunk &other) const
    {
        return origin == other.origin;
    }

private:
    // IDK what to put here yet
};

namespace std
{
    template <>
    struct hash<Chunk>
    {
        size_t operator()(const Chunk &chunk) const
        {
            size_t h1 = hash<float>()(chunk.origin.x);
            size_t h2 = hash<float>()(chunk.origin.y);
            size_t h3 = hash<float>()(chunk.origin.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); // Combine hashes
        }
    };
}

#endif