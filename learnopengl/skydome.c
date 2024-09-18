#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define PI 3.14159265358979323846

// Vertex shader
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec3 TexCoords;\n"
    "void main()\n"
    "{\n"
    "    TexCoords = aPos;\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\0";

// Fragment shader
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 TexCoords;\n"
    "uniform bool showSky;\n"
    "uniform bool showWater;\n"
    "void main()\n"
    "{\n"
    "    vec3 skyColor = vec3(0.5, 0.7, 1.0);\n"
    "    vec3 waterColor = vec3(0.0, 0.2, 0.5);\n"
    "    vec3 horizonColor = vec3(0.8, 0.8, 0.9);\n"
    "    \n"
    "    float t = (TexCoords.y + 1.0) * 0.5;\n"
    "    if (TexCoords.y > 0.0) {\n"
    "        FragColor = vec4(mix(horizonColor, skyColor, pow(t, 0.5)), 1.0);\n"
    "    } else {\n"
    "        FragColor = vec4(mix(horizonColor, waterColor, pow(1.0 - t, 0.5)), 1.0);\n"
    "    }\n"
    "}\0";

// Function to create a sphere mesh
void createSphereMesh(float radius, int stacks, int slices, float** vertices, int* vertexCount) {
    *vertexCount = (stacks + 1) * (slices + 1) * 2;
    *vertices = (float*)malloc(*vertexCount * 3 * sizeof(float));
    
    int index = 0;
    for (int i = 0; i <= stacks; ++i) {
        float phi1 = PI * i / (float)stacks;
        float phi2 = PI * (i + 1) / (float)stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2 * PI * j / (float)slices;
            
            float x1 = radius * sinf(phi1) * cosf(theta);
            float y1 = radius * cosf(phi1);
            float z1 = radius * sinf(phi1) * sinf(theta);
            
            float x2 = radius * sinf(phi2) * cosf(theta);
            float y2 = radius * cosf(phi2);
            float z2 = radius * sinf(phi2) * sinf(theta);
            
            (*vertices)[index++] = x1;
            (*vertices)[index++] = y1;
            (*vertices)[index++] = z1;
            
            (*vertices)[index++] = x2;
            (*vertices)[index++] = y2;
            (*vertices)[index++] = z2;
        }
    }
}

// Camera structure
typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
} Camera;

// Global variables
Camera camera = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, -90.0f, 0.0f};
bool showSky = true;
bool showWater = true;
float lastX = 400, lastY = 300;
bool firstMouse = true;
bool mouseCaptured = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseCaptured) return;
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.yaw   += xoffset;
    camera.pitch += yoffset;

    if(camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if(camera.pitch < -89.0f)
        camera.pitch = -89.0f;

    vec3 front;
    front[0] = cos(glm_rad(camera.yaw)) * cos(glm_rad(camera.pitch));
    front[1] = sin(glm_rad(camera.pitch));
    front[2] = sin(glm_rad(camera.yaw)) * cos(glm_rad(camera.pitch));
    glm_normalize(front);
    glm_vec3_copy(front, camera.front);
}

void processInput(GLFWwindow *window) {
    float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        glm_vec3_muladds(camera.front, cameraSpeed, camera.position);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        glm_vec3_muladds(camera.front, -cameraSpeed, camera.position);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 cross;
        glm_vec3_cross(camera.front, camera.up, cross);
        glm_normalize(cross);
        glm_vec3_muladds(cross, -cameraSpeed, camera.position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 cross;
        glm_vec3_cross(camera.front, camera.up, cross);
        glm_normalize(cross);
        glm_vec3_muladds(cross, cameraSpeed, camera.position);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        showSky = !showSky;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        showWater = !showWater;
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        mouseCaptured = !mouseCaptured;
        if (mouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Enhanced Skydome", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Create and compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create sphere mesh
    float* vertices;
    int vertexCount;
    createSphereMesh(10.0f, 64, 64, &vertices, &vertexCount);

    // Create VAO and VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set polygon mode to fill
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up projection and view matrices
        mat4 projection, view, model;
        glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);
        glm_lookat(camera.position, (vec3){camera.position[0] + camera.front[0], 
                                           camera.position[1] + camera.front[1], 
                                           camera.position[2] + camera.front[2]}, 
                   camera.up, view);
        glm_mat4_identity(model);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (float*)projection);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (float*)model);
        
        // Set uniforms for sky and water visibility
        glUniform1i(glGetUniformLocation(shaderProgram, "showSky"), showSky);
        glUniform1i(glGetUniformLocation(shaderProgram, "showWater"), showWater);

        glBindVertexArray(VAO);
        for (int i = 0; i < vertexCount - 2; i += 2) {
    glDrawArrays(GL_TRIANGLE_STRIP, i, 4);
}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    free(vertices);

    glfwTerminate();
    return 0;
}
