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
#include <vector>

#include "block.h"

using namespace std;

const int AIR   = 0;
const int GRASS = 1;
const int DIRT  = 2;
const int SAND  = 3;
const int TREE  = 4;
const int LEAF  = 5;
const int WATER = 6;

// Forward declarations for noise functions
float perlin(float x, float y);
float dotGridGradient(int ix, int iy, float x, float y);
float interpolate(float a0, float a1, float w);
glm::vec2 randomGradient(int ix, int iy);
float fbm(float x, float y, int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

std::random_device rd; 
std::mt19937 gen(rd());

// Basic pseudo-random gradient generation based on grid coordinates
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
    float random = a * (3.14159265f / ~(~0u >> 1)); // angle in [0,2*pi]
    glm::vec2 v;
    v.x = sin(random);
    v.y = cos(random);
    return v;
}

float dotGridGradient(int ix, int iy, float x, float y)
{
    glm::vec2 gradient = randomGradient(ix, iy);
    float dx = x - (float)ix;
    float dy = y - (float)iy;
    return (dx * gradient.x + dy * gradient.y);
}

float interpolate(float a0, float a1, float w)
{
    // Using smoothstep interpolation (3-2w)*w^2
    return (a1 - a0) * (3.0f - 2.0f * w) * w * w + a0;
}

float perlin(float x, float y)
{
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;

    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);

    float value = interpolate(ix0, ix1, sy);
    return glm::clamp(value, -1.0f, 1.0f);
}

float fbm(float x, float y, int octaves, float persistence, float lacunarity)
{
    float total = 0.0f;
    float amplitude = 0.5f;
    float frequency = 0.5f;
    float maxValue = 0.0f;
    for (int i = 0; i < octaves; i++)
    {
        total += perlin(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total / maxValue; // roughly normalized to [-1,1]
}

class Chunk
{
public:
    // Chunk dimensions: 16 x 16 x 16
    static const unsigned int CHUNK_SIZE = 16;
    // Container for blocks
    std::vector<Block> blocks;
    // Vectors for trees and leaves
    std::vector<Block> trees;
    std::vector<Block> leaves;
    // World-space origin of the chunk
    glm::vec3 origin;

    Chunk(glm::vec3 originVector)
    {
        origin = originVector;
        // Adjust noiseScaler to control horizontal feature size.
        float noiseScaler = 0.03f;
        // Set maximum terrain height within the bounds 0 to CHUNK_SIZE - 1.
        float maxTerrainHeight = (float)CHUNK_SIZE - 1;

        // Precompute fbm noise for each (x, z) coordinate in the chunk.
        vector<vector<float>> noiseValues(CHUNK_SIZE, vector<float>(CHUNK_SIZE, 0.0f));
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                float rawNoise = fbm((x + origin.x) * noiseScaler, (z + origin.z) * noiseScaler, 4, 0.7f, 1.7f);
                // Normalize the raw noise from [-1,1] to [0,1]
                float normalizedNoise = (rawNoise + 1.0f) / 2.0f;
                // Apply contrast to accentuate differences while still clamping between 0 and 1.
                float contrast = 1.2f;  // Increase this value for greater variation
                normalizedNoise = glm::clamp((normalizedNoise - 0.5f) * contrast + 0.5f, 0.0f, 1.0f);
                noiseValues[x][z] = normalizedNoise;
            }
        }

        // Build the chunk by iterating over (x,z) and then y.
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                float normalizedNoise = noiseValues[x][z];
                // Scale noise to the maximum terrain height.
                float terrainHeightF = normalizedNoise * maxTerrainHeight;
                int terrainHeight = (int)floor(terrainHeightF);

                for (int y = 0; y < CHUNK_SIZE; y++)
                {
                    int index = (x * CHUNK_SIZE * CHUNK_SIZE) + (y + (z * CHUNK_SIZE));
                    // Choose block type based on vertical position relative to the terrain height.
                    if (y == terrainHeight && y > 3)
                    {
                        blocks.push_back(Block(glm::vec3(x + origin.x, y + origin.y, z + origin.z), GRASS));
                    }
                    else if (y < terrainHeight && y > 3)
                    {
                        blocks.push_back(Block(glm::vec3(x + origin.x, y + origin.y, z + origin.z), DIRT));
                    }
                    else if (y <= terrainHeight && y == 3)
                    {
                        blocks.push_back(Block(glm::vec3(x + origin.x, y + origin.y, z + origin.z), SAND));
                    }
                    else if ((y < terrainHeight && y < 3) || (y <= 1 && terrainHeight <= 1) || y < 3)
                    {
                        blocks.push_back(Block(glm::vec3(x + origin.x, y + origin.y, z + origin.z), WATER));
                    }
                    else
                    {
                        blocks.push_back(Block(glm::vec3(x + origin.x, y + origin.y, z + origin.z), AIR));
                    }

                    // Tree and water logic for where there is gradd
                    if (blocks.at(index).blockType == GRASS)
                    {
                         float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                        // Adjust the probability as desired
                        if (r < 0.02f && r > 0.01f) 
                        {
                            // Define tree parameters.
                            const int trunkHeight = 4;   
                            const int crownRadius = 2;  

                            // Build the trunk.
                            for (int i = 0; i < trunkHeight; i++)
                            {
                                trees.push_back(Block(glm::vec3(x + origin.x, y + 1 + i + origin.y, z + origin.z), TREE));
                            }
                            
                            // Define the center of the crown as the top of the trunk.
                            glm::vec3 crownCenter(x + origin.x, y + trunkHeight + origin.y, z + origin.z);

                            // Generate a spherical crown of leaves in a symmetrical pattern.
                            for (int dx = -crownRadius; dx <= crownRadius; dx++)
                            {
                                for (int dy = -crownRadius + 1; dy <= crownRadius; dy++)
                                {
                                    for (int dz = -crownRadius; dz <= crownRadius; dz++)
                                    {
                                        // Adjust the threshold (radius + 0.5f) for rounding if desired.
                                        if (glm::length(glm::vec3(dx, dy, dz)) <= crownRadius + 0.5f)
                                        {
                                            leaves.push_back(Block(crownCenter + glm::vec3(dx, dy, dz), LEAF));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    glm::vec3 GetOrigin()
    {
        return origin;
    }

    bool operator==(const Chunk &other) const
    {
        return origin == other.origin;
    }
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
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

#endif
