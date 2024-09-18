#include <raylib.h>
#include <GL/glew.h>
#include <cglm/cglm.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>

// Function prototypes
char* loadShaderSource(const char* filename);
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
void setupShaders();
void setupVBO();
void setupMatrices();
void drawFrame();

// VBO and shader program
GLuint vbo, vao, shaderProgram;
GLuint modelLoc, viewLoc, projectionLoc;
mat4 modelMatrix, viewMatrix, projectionMatrix;


char* loadShaderSource(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        printf("Failed to allocate memory for shader source\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
        return 0;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (!vertexShader || !fragmentShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void setupShaders() {
    char* vertexShaderSource = loadShaderSource("vertex.glsl");
    char* fragmentShaderSource = loadShaderSource("fragment.glsl");

    if (!vertexShaderSource || !fragmentShaderSource) {
        printf("Failed to load shader sources\n");
        return;
    }

    printf("Vertex Shader Source:\n%s\n", vertexShaderSource);
    printf("Fragment Shader Source:\n%s\n", fragmentShaderSource);

    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    free(vertexShaderSource);
    free(fragmentShaderSource);

    if (!shaderProgram) {
        printf("Failed to create shader program\n");
        return;
    }

    printf("Shader program created successfully\n");

    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    printf("Uniform locations: model=%d, view=%d, projection=%d\n", modelLoc, viewLoc, projectionLoc);
}


void setupVBO() {
    // Define vertices for a simple colored cube
    GLfloat vertices[] = {
        // Positions         // Colors
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f
    };
    GLuint indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
        0, 3, 7, 7, 4, 0,
        1, 2, 6, 6, 5, 1
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void setupMatrices() {
    glm_mat4_identity(modelMatrix);
    glm_translate(modelMatrix, (vec3){0.0f, 0.0f, -2.0f});
    glm_lookat((vec3){4.0f, 4.0f, 4.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, viewMatrix);
    glm_perspective(glm_rad(45.0f), 800.0f/600.0f, 0.1f, 100.0f, projectionMatrix);
}

void drawFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)modelMatrix);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)viewMatrix);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projectionMatrix);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}


void setup() {
    InitWindow(800, 600, "Shader + VBO + WebGL2 Example");
    setupShaders();
    setupVBO();
    setupMatrices();
    glEnable(GL_DEPTH_TEST);
}

int main() {
    setup();
    emscripten_set_main_loop(drawFrame, 0, 1);
    CloseWindow();
    return 0;
}
