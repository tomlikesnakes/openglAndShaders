#include "Utils.h"
#include <math.h>
#include <stdlib.h>

#define numVAOs 1
#define numVBOs 1


float cameraX, cameraY, cameraZ;
float cubeLocX, cubeLocY, cubeLocZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// Allocate variables used in display() function, so that they won't need to be allocated during rendering
//GLuint mvLoc, projLoc;
int width, height, displayLoopi;
float aspect;
float timeFactor;
GLuint mvLoc, projLoc, tfLoc;
float pMat[16], vMat[16], tMat[16], rMat[16], mMat[16], mvMat[16];

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
    // position the camera further down the positive Z axis (to see all of the cubes)
    cameraX = 0.0f; cameraY = 0.0f; cameraZ = 420.0f;

    setupVertices();
}


void display(GLFWwindow* window, double currentTime) {
        glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);

    // Get uniform locations for MV and projection matrices
    mvLoc = glGetUniformLocation(renderingProgram, "v_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

    // Get framebuffer size and set the OpenGL viewport to match
    glfwGetFramebufferSize(window, &width, &height);
    //glViewport(0, 0, width, height);

    // Calculate aspect ratio and set perspective projection
    aspect = (float)width / (float)height;
    utils_matrix_perspective(pMat, 1.0472f, aspect, 0.1f, 1000.0f);  // Adjust FOV to 45 degrees

    // Move the camera back further to ensure cubes are visible
    cameraZ = 420.0f;

    // Initialize identity matrices for view, model, and combined model-view
    utils_matrix_identity(vMat);
    utils_matrix_translate(vMat, -cameraX, -cameraY, -cameraZ);



    // Copy perspective and model-view matrices to corresponding uniform variables
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, (float*)mvMat);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)pMat);

    timeFactor = ((float)currentTime);
    tfLoc = glGetUniformLocation(renderingProgram, "tf");
    glUniform1f(tfLoc, (float)timeFactor);

    // Associate VBO with the corresponding vertex attribute in the vertex shader
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Enable depth testing and render the cube
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 100000);
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
