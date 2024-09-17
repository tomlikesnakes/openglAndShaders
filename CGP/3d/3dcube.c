#include <cglm/cglm.h>
#include "Utils.h"
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define numVAOs 1
#define numVBOs 1

float cameraX, cameraY, cameraZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Allocate variables used in display() function
int width, height;
float aspect;
float timeFactor;
GLuint mvLoc, projLoc, tfLoc;
mat4 pMat, vMat, mMat, mvMat;

void setupVertices(void) {
    float vertexPositions[108] = {
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
    };

    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
}

void init(GLFWwindow* window) {
    renderingProgram = create_shader_program("shaders/vertShader.glsl", "shaders/fragShader.glsl");
    cameraX = 0.0f; cameraY = 0.0f; cameraZ = 420.0f;
    setupVertices();
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);

    // Get uniform locations
    mvLoc = glGetUniformLocation(renderingProgram, "v_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

    // Get framebuffer size and set viewport
    glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);  // Set the viewport to cover the entire window

    aspect = (float)width / (float)height;

    // Set up perspective matrix using cglm
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 1000.0f, pMat);

    // Set up view matrix: translate the camera position
    glm_mat4_identity(vMat);
    glm_translate(vMat, (vec3){-cameraX, -cameraY, -cameraZ});

    // Set uniforms
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)pMat);
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, (float*)vMat);

    // Set time factor for animation
    timeFactor = (float)currentTime;
    tfLoc = glGetUniformLocation(renderingProgram, "tf");
    glUniform1f(tfLoc, timeFactor);

    // Bind VBO and set up vertex attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Enable depth testing and render the cubes
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 100000);
}

int main(void) {
    if (!glfwInit()) { exit(EXIT_FAILURE); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(600, 600, "3D Cubes with cglm", NULL, NULL);
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

