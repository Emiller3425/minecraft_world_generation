#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <math.h>

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

    glm::vec2 randomGradient(int ix, int iy) {
        const unsigned w = 8 * sizeof(unsigned);
        const unsigned s = w / 2;
        unsigned a = ix, b = iy;
        a *= 3284157443;

        b ^= a << s | a > w - s;
        b *= 1911520717;

        a ^= b << s | b >> w - s;
        b *= 2048419325;
        float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*pi]

        glm::vec2 v;
        v.x = sin(random);
        v.y = cos(random);

        return v;
    }

    // Computes the dot product of the distance of gradient vecotrs.
    float dotGridGradient(int ix, int iy, float x, float y) {
        glm::vec2 gradient = randomGradient(ix, iy);

        // Compute the distance vector
        float dx = x - (float)ix;
        float dy = y - (float)iy;

        // Compute the dot-product
        return (dx * gradient.x + dy * gradient.y);
    }
    
    float interpolate(float a0, float a1, float w) {
        return (a1 -a0) * (3.0f - w * 2.0f) * w * w + a0;
    }

    // Sample Perlin noise at coordinates x, y
    float perlin(float x, float y) {
        
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

        return value;
    }

    glm::vec3 GetOrigin() {
        return origin;
    }
    private:
    // IDK what to put here yet
};

#endif