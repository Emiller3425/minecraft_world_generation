/**
 * TODO:
 * 
 * Multi-threading for mesh and chunk creation -- make the chunk creation on a seperate thread to avoid the freezing -- also needs to be more efficient
 * 
 * Then and only then increase the render range now that we are only passing new chunks when updating the mesh
 * 
 */

#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include "headers/shader.h"
#include "headers/stb_image.h"
#include "headers/camera.h"
#include "headers/chunk.h"
#include "headers/mesh.h"
#include "headers/block.h"


using namespace std;

//structs
struct Plane {
    glm::vec3 normal; // plane
    float distance;  // distance from origin
};

// might use this might not
struct Frustrum {
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane frontFace;
    Plane rearFace;
};

// buffer call back and input funciton definitions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void generateBindTextures(unsigned int &texture, const char *path);
unsigned int loadCubemap(vector<std::string> faces);
void drawCube(unsigned int textures[]);
void drawSkybox(unsigned int cubemapTextureID);
void checkNewChunks(glm::vec3 playerPos, unordered_set<Chunk>& chunks, Mesh& mesh);
Frustrum createFrustrumFromCamera(const Camera& camera, float aspect, float fovY, float zNear, float zFar);
bool isCubeInFrustrum(const Frustrum& frustum, const glm::vec3& cubeCenter, float radius);


// window size
const unsigned int SRC_WIDTH = 800;
const unsigned int SRC_HEIGHT = 600;

// camera shit
Camera camera(glm::vec3(8.0f, 20.0f, 8.0f));

// planes for frustrum
Plane topFace;
Plane bottomFace;
Plane rightFace;
Plane leftFace;
Plane frontFace;
Plane rearFace;

// frustrum
Frustrum frustrum;

float lastX = 400, lastY = 300;
bool firstMouse = true;

// delta time
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main()
{
    /*   Initializing OPENGL, Creating and defining a window  ************************************************************************************************************* */
    // Initialize GLFW
    if (!glfwInit())
    {
        cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // forward compat is required for mac os
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Initialize window
    GLFWwindow *window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "Fuck Me", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create window" << endl;
        glfwTerminate();
        return -1;
    }
    // set window to current context
    glfwMakeContextCurrent(window);

    // Register the framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Hide cursor and capture it
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // set cursor callback
    glfwSetCursorPosCallback(window, mouse_callback);

    // set scroll callback
    glfwSetScrollCallback(window, scroll_callback);

    // initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize glad" << endl;
        return -1;
    }
    // define shaders
    Shader textureShader("shaders/texture.vs", "shaders/texture.fs");
        
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");

    // tell openGL the size of the window
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    topFace.normal = camera.Position + glm::vec3(0.0f, 4.0f, 0.0f);
    topFace.distance = 4.0f;
    bottomFace.normal = camera.Position + glm::vec3(0.0f, -4.0f, 0.0f);
    bottomFace.distance = 4.0f;
    rightFace.normal = camera.Position + glm::vec3(4.0f, 0.0f, 0.0f);
    rightFace.distance = 4.0f;
    leftFace.normal = camera.Position + glm::vec3(-4.0f, 0.0f, 0.0f);
    leftFace.distance = 4.0f;
    frontFace.normal = camera.Position + glm::vec3(0.0f, 0.0f, 4.0f);
    frontFace.distance = 4.0f;
    rearFace.normal = camera.Position + glm::vec3(0.0f, 0.0f, -4.0f);
    rearFace.distance = 4.0f;


    // define frustrum faces
    frustrum.topFace = topFace;
    frustrum.bottomFace = bottomFace;
    frustrum.rightFace = rightFace;
    frustrum.leftFace = leftFace;
    frustrum.frontFace = frontFace;
    frustrum.rearFace = rearFace;

    // the vetices are laid out by the x,y,z positions, then the next 2 lay out how the texture is mapped to the vertices. the last 3 then represent the normal vector perpendicular to each plane the groups of vertices represent.
    float vertices[] = {
        // Face -Z (front)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        // Face +Z (back)
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // Face -X (left)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        // Face +X (right)
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        // Face -Y (bottom)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,

        // Face +Y (top)
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};


    // skybox verticles are just the x, y, z positions
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    // only render outside plane of textures, not the inside
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind vertex arrray object first
    glBindVertexArray(VAO);
    // bind bufffer object to buffer target
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLsizei stride = (3 + 2 + 3) * sizeof(float);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal vector attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)((3 + 2) * sizeof(float)));
    glEnableVertexAttribArray(2);


    // skybox VBO
    unsigned int skyboxVBO, skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    // bind vertex array object first
    glBindVertexArray(skyboxVAO);
    // bind buffer to target
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // WIREFRAME DRAWING
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // generate and bind texture id
    unsigned int texture1 = 0, texture2 = 0, texture3 = 0;
    unsigned int grass_textures[3] = {texture1, texture2, texture3};
    for (int i = 0; i < 3; i++)
    {
        string path = "graphics/grass_block/" + to_string(i) + ".png";
        generateBindTextures(grass_textures[i], path.c_str());
    }
    unsigned int texture4 = 0;
    unsigned int dirt_textures[1] = {texture4};
    for (int i = 0; i < 3; i++)
    {
        string path = "graphics/dirt_block/" + to_string(i) + ".png";
        generateBindTextures(dirt_textures[i], path.c_str());
    }

    unsigned int texture5 = 0;
    unsigned int sand_textures[1] = {texture5};
    for (int i = 0; i < 3; i++)
    {
        string path = "graphics/sand_block/" + to_string(i) + ".png";
        generateBindTextures(sand_textures[i], path.c_str());
    }

    unsigned int texture6 = 0;
    unsigned int tree_textures[1] = {texture6};
    for (int i = 0; i < 3; i++)
    {
        string path = "graphics/tree_block/" + to_string(i) + ".png";
        generateBindTextures(tree_textures[i], path.c_str());
    }

    unsigned int texture7 = 0;
    unsigned int leaf_textures[1] = {texture7};
    for (int i = 0; i < 3; i++)
    {
        string path = "graphics/leaf_block/" + to_string(i) + ".png";
        generateBindTextures(leaf_textures[i], path.c_str());
    }

    unsigned int texture8 = 0;
    unsigned int water_textures[1] = {texture8};
    for (int i = 0; i < 3; i++)
    {
        string path = "graphics/water_block/" + to_string(i) + ".png";
        generateBindTextures(water_textures[i], path.c_str());
    }

    vector<std::string> skyboxFaces {
        "graphics/skybox/0.png",
        "graphics/skybox/0.png",
        "graphics/skybox/0.png",
        "graphics/skybox/0.png",
        "graphics/skybox/0.png",
        "graphics/skybox/0.png",
    };

    unsigned int cubemapTexture = loadCubemap(skyboxFaces);

    // check that we were able to create cubemap texture
    if (cubemapTexture == 0) {
        cerr << "Failed to load cubemap texture. Exiting." << endl;
        glfwTerminate();
        return -1;
    }

    // define list of chunks
    std::unordered_set<Chunk> chunks;

    // define light position
    glm::vec3 lightPosition = glm::vec3(12.0f, 60.0f, -12.0f);

    // define mesh
    Mesh mesh(chunks);

       while (!glfwWindowShouldClose(window))
    {
        // calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // process input
        processInput(window);
        glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
        glEnable(GL_DEPTH_TEST); // Ensure depth testing is enabled before clearing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Check for new chunks to load/mesh
        checkNewChunks(camera.Position, chunks, mesh);

        // Update frustum
        frustrum = createFrustrumFromCamera(camera, (float)SRC_WIDTH / (float)SRC_HEIGHT, camera.Zoom, 0.1f, 300.0f);

        // Common matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();


        // --- Render Skybox ---
        glDepthFunc(GL_LEQUAL);      // Change depth function so fragments equal to depth buffer value pass (skybox sits at far plane)
        glDepthMask(GL_FALSE);     // Disable writing to the depth buffer for the skybox

        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // Remove translation from the view matrix
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setInt("skybox", 0); // Use texture unit 0

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); // Bind the cubemap texture
        glDrawArrays(GL_TRIANGLES, 0, 36); // Draw the skybox
        glBindVertexArray(0); // Unbind skybox VAO


        // --- Restore default state for world rendering ---
        glDepthMask(GL_TRUE);      // Re-enable depth writing **
        glDepthFunc(GL_LESS);      // Restore default depth function
        glEnable(GL_CULL_FACE);    // Ensure face culling is enabled ** (If you disabled it for skybox)
        glCullFace(GL_BACK);       // Restore back-face culling **


        // --- Render World Geometry ---
        textureShader.use();
        textureShader.setMat4("view", view); // Use the normal view matrix
        textureShader.setMat4("projection", projection);
        textureShader.setInt("texture1", 0); // Tell world shader sampler "texture1" to use texture unit 0
        textureShader.setVec3("lightPos", lightPosition);
        textureShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

        glBindVertexArray(VAO); // Bind world geometry VAO

        // render opaque textures first
        for (const auto &renderCube : mesh.renderOpaqueCubes) {
            // Frustum check (using the provided radius)
            if (!isCubeInFrustrum(frustrum, renderCube.blockPosition + glm::vec3(0.5f), 300.0f)) continue;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, renderCube.blockPosition);
            textureShader.setMat4("model", model);

            // Select and bind appropriate 2D texture based on block type
            glActiveTexture(GL_TEXTURE0);
            if (renderCube.blockType == GRASS) {
                drawCube(grass_textures);
            } else if (renderCube.blockType == DIRT) {
                drawCube(dirt_textures);
            }  else if (renderCube.blockType == TREE) {
                drawCube(tree_textures);
            } else if (renderCube.blockType == WATER) {
                 drawCube(water_textures);
            }else if (renderCube.blockType == SAND){
                drawCube(sand_textures);
            }
        }

        // Render transparent cubes afterwards
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto &renderCube : mesh.renderTransparentCubes) {
             if (!isCubeInFrustrum(frustrum, renderCube.blockPosition + glm::vec3(0.5f), 300.0f)) continue;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, renderCube.blockPosition);
            textureShader.setMat4("model", model);

            glActiveTexture(GL_TEXTURE0); 

            if (renderCube.blockType == LEAF) { // Transparent leaves
                drawCube(leaf_textures);
            }
        }

        glBindVertexArray(0); // Unbind world VAO

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // terminate glfw de-allocating all used resources
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void generateBindTextures(unsigned int &texture, const char *path)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate textures
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    { // handle color GL color format
        GLenum format = 0;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        cout << "Failed to load texture\n";
    }
    stbi_image_free(data);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false); // Cubemaps often don't need flipping
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = GL_RGB; // Assuming RGB, adjust if your images have alpha
             if (nrChannels == 1)
                 format = GL_RED;
             else if (nrChannels == 3)
                 format = GL_RGB;
             else if (nrChannels == 4)
                 format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, // Target face (+X, -X, +Y, ...)
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data); // Even if data is null, free attempts are safe
             glDeleteTextures(1, &textureID); // Clean up texture ID if loading fails
             return 0; // Return 0 to indicate failure
        }
    }
    stbi_set_flip_vertically_on_load(true); // Set back to true for other textures if needed

    // Set cubemap texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind

    return textureID;
}

void drawCube(unsigned int textures[])
{
    // 2) Draw the four side faces  (indices 0..23) with 0.png
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 24);

    // 3) Draw the bottom face (indices 24..29) with 2.png
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glDrawArrays(GL_TRIANGLES, 24, 6);

    // 4) Draw the top face (indices 30..35) with 1.png
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glDrawArrays(GL_TRIANGLES, 30, 6);
}

void drawSkybox(unsigned int cubemapTextureID) {
    // Bind the single cubemap texture ID
    glActiveTexture(GL_TEXTURE0); // Ensure texture unit 0 is active (usually default)
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
    // The VAO should be bound before calling this function
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void checkNewChunks(glm::vec3 playerPos, unordered_set<Chunk>& chunks, Mesh& mesh){
    // current x and y chunk
    float x_chunk = (float)floor(playerPos.x / 16);
    float z_chunk = (float)floor(playerPos.z / 16);
    unordered_set<Chunk> newChunks;
    // use for when loop is created to check a larger radius
    int renderRange = 10;

    // check if surrounding chunks exist
    int chunk_amount = newChunks.size();
        if (chunks.find(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, z_chunk * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, z_chunk * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, z_chunk * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk + 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk - 1) * Chunk::CHUNK_SIZE)));
        }
        if (chunks.find(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE))) == chunks.end()) {
            chunks.insert(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE)));
            newChunks.insert(Chunk(glm::vec3((x_chunk - 1) * Chunk::CHUNK_SIZE, 0.0f, (z_chunk + 1) * Chunk::CHUNK_SIZE)));
        }
        // if we added a new chunk, edit the mesh
        if (newChunks.size() > chunk_amount) {
            mesh.addChunksToMesh(newChunks); // we only want to call this once, we need to track size before and after insert
        }
        newChunks.clear();
}

Frustrum createFrustrumFromCamera(const Camera& camera, float aspect, float fovY, float zNear, float zFar) {
    Frustrum frustrum;

    // Far plane size
    const float halfVSide = zFar * tanf(glm::radians(fovY) * 0.5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * camera.Front;

    // Near plane
    glm::vec3 nearCenter = camera.Position + camera.Front * zNear;
    frustrum.frontFace.normal = camera.Front;
    frustrum.frontFace.distance = -glm::dot(frustrum.frontFace.normal, nearCenter);

    // Far plane
    glm::vec3 farCenter = camera.Position + frontMultFar;
    frustrum.rearFace.normal = -camera.Front;
    frustrum.rearFace.distance = -glm::dot(frustrum.rearFace.normal, farCenter);

    // Right plane
    glm::vec3 rightPoint = camera.Position + frontMultFar - camera.Right * halfHSide;
    glm::vec3 rightNormal = glm::normalize(glm::cross(camera.Up, rightPoint - camera.Position));
    frustrum.rightFace.normal = rightNormal;
    frustrum.rightFace.distance = -glm::dot(rightNormal, camera.Position);

    // Left plane
    glm::vec3 leftPoint = camera.Position + frontMultFar + camera.Right * halfHSide;
    glm::vec3 leftNormal = glm::normalize(glm::cross(leftPoint - camera.Position, camera.Up));
    frustrum.leftFace.normal = leftNormal;
    frustrum.leftFace.distance = -glm::dot(leftNormal, camera.Position);

    // Top plane
    glm::vec3 topPoint = camera.Position + frontMultFar - camera.Up * halfVSide;
    glm::vec3 topNormal = glm::normalize(glm::cross(camera.Right, topPoint - camera.Position));
    frustrum.topFace.normal = topNormal;
    frustrum.topFace.distance = -glm::dot(topNormal, camera.Position);

    // Bottom plane
    glm::vec3 bottomPoint = camera.Position + frontMultFar + camera.Up * halfVSide;
    glm::vec3 bottomNormal = glm::normalize(glm::cross(bottomPoint - camera.Position, camera.Right));
    frustrum.bottomFace.normal = bottomNormal;
    frustrum.bottomFace.distance = -glm::dot(bottomNormal, camera.Position);

    return frustrum;
}

bool isCubeInFrustrum(const Frustrum& frustum, const glm::vec3& center, float radius) {
    Plane planes[6] = {
        frustum.frontFace,
        frustum.rearFace,
        frustum.leftFace,
        frustum.rightFace,
        frustum.topFace,
        frustum.bottomFace
    };

    for (int i = 0; i < 6; i++) {
        // Dot product to check distance from the plane
        float distance = glm::dot(planes[i].normal, center) + planes[i].distance;
        if (distance < -radius) {
            // The cube is completely outside this plane
            return false;
        }
    }
    return true;
}
