#include "Utils.h"
#include <math.h>
#include <stdlib.h>

#define numVAOs 1
#define numVBOs 2

float cameraX, cameraY, cameraZ;
float cubeLocX, cubeLocY, cubeLocZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Allocate variables used in display() function, so that they won't need to be allocated during rendering
GLuint mvLoc, projLoc;
int width, height;
float aspect;
float pMat[16], vMat[16], mMat[16], mvMat[16];

void setupVertices(void) {
    float vertexPositions[108] = {
        -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f
    };

    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
}

void init(GLFWwindow* window) {
    renderingProgram = create_shader_program("shaders/vertShader.glsl", "shaders/fragShader.glsl");
    cameraX = 0.0f; cameraY = 0.0f; cameraZ = 8.0f;
    cubeLocX = 0.0f; cubeLocY = -2.0f; cubeLocZ = 0.0f;  // shift down Y to reveal perspective
    setupVertices();
}

void display(GLFWwindow* window, double currentTime) {
    // Clear buffers
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);

    // Get uniform locations for MV and projection matrices
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

       // Get the framebuffer size (instead of window size) to handle high-DPI monitors and i3 properly
    glfwGetFramebufferSize(window, &width, &height);
    
    // Ensure the OpenGL viewport matches the framebuffer size
    glViewport(0, 0, width, height);  // Set the viewport to cover the entire window

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);

    // Calculate aspect ratio and set perspective projection
    aspect = (float)width / (float)height;

    // Adjust the field of view to ensure cubes fill more of the screen (you can tweak this value)
    utils_matrix_perspective(pMat, 0.7854f, aspect, 0.1f, 1000.0f);  // 45 degrees FOV

    // Move the camera further back to give the cubes more space
    cameraZ = 30.0f;

    // Loop over 24 cubes and apply transformations
    for (int i = 0; i < 48; i++) {
        // Initialize identity matrices for view, model, and combined model-view
        utils_matrix_identity(vMat);
        utils_matrix_translate(vMat, -cameraX, -cameraY, -cameraZ);

        // Translation based on currentTime and loop index i
        utils_matrix_identity(mMat);  // Initialize model matrix to identity
        utils_matrix_translate(mMat, 
            sinf(0.35f * currentTime + i) * 10.0f,  // Increase translation factor for larger spread
            cosf(0.52f * currentTime + i) * 10.0f,
            sinf(0.7f * currentTime + i) * 10.0f
        );

        // Apply rotations to the model matrix (rotation based on currentTime)
        utils_matrix_rotate(mMat, 1.75f * currentTime, 0.0f, 1.0f, 0.0f); // Y-axis rotation
        utils_matrix_rotate(mMat, 1.75f * currentTime, 1.0f, 0.0f, 0.0f); // X-axis rotation
        utils_matrix_rotate(mMat, 1.75f * currentTime, 0.0f, 0.0f, 1.0f); // Z-axis rotation

        // Multiply the view matrix (vMat) with the model matrix (mMat) to get the model-view matrix
        utils_matrix_multiply(vMat, mMat, mvMat);

        // Copy perspective and model-view matrices to corresponding uniform variables
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, (float*)mvMat);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)pMat);

        // Associate VBO with the corresponding vertex attribute in the vertex shader
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // Enable depth testing and render the cube
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}


int main(void) {
    if (!glfwInit()) { exit(EXIT_FAILURE); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter 4 - program 1", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
    glfwSwapInterval(1);

    init(window);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
