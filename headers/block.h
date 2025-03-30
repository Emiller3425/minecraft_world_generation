#ifndef BLOCK_H
#define BLOCK_H

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

class Block {
public:
    glm::vec3 blockPosition;
    int blockType;

    Block() : blockPosition(glm::vec3(0.0f)), blockType(0) {}

    Block(glm::vec3 position, int typeValue) {
        blockPosition = glm::vec3(position);
        blockType = typeValue;
    }

    // Overload equality operator for unordered_set
    bool operator==(const Block& other) const {
        return blockPosition == other.blockPosition &&
        blockType == other.blockType;
    }
    private:
    // private method
};

// Custom hash function for glm::vec3
namespace std {
    template <>
    struct hash<glm::vec3> {
        size_t operator()(const glm::vec3& v) const {
            return hash<float>()(v.x) ^ hash<float>()(v.y) ^ hash<float>()(v.z);
        }
    };

    template <>
    struct hash<Block> {
        size_t operator()(const Block &block) const {
            // Compute hashes for each component of blockPosition
            size_t h1 = std::hash<float>()(block.blockPosition.x);
            size_t h2 = std::hash<float>()(block.blockPosition.y);
            size_t h3 = std::hash<float>()(block.blockPosition.z);
            // Combine the hash values. There are many ways to combine hashes;
            // here we use a simple XOR and bit shifting.
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

#endif