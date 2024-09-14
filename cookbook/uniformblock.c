#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/cglm.h>

GLuint prog;  // Assuming this is a global program handle
GLuint vaoHandle;
float angle = 0.0f;
int width, height;

void compile();
void initUniformBlockBuffer();
void initScene();
void update(float t);
void render();
void resize(int w, int h);

GLuint compileShader(const char* filepath, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open shader file %s\n", filepath);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *source = (char *)malloc(fileSize + 1);
    fread(source, 1, fileSize, file);
    source[fileSize] = '\0';
    fclose(file);

    glShaderSource(shader, 1, (const GLchar* const*)&source, NULL);
    free(source);

    glCompileShader(shader);
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Error: Shader compilation failed\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    return shader;
}

void compile() {
    GLuint vertexShader = compileShader("shader/basic_uniformblock.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader("shader/basic_uniformblock.frag", GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint linkStatus;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "Error: Shader linking failed\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    prog = shaderProgram; // Assign the program handle
}

void initUniformBlockBuffer() {
    GLuint programHandle = prog;
    GLuint blockIndex = glGetUniformBlockIndex(programHandle, "BlobSettings");

    GLint blockSize;
    glGetActiveUniformBlockiv(programHandle, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    GLubyte *blockBuffer = (GLubyte *)malloc(blockSize);

    const GLchar *names[] = {
        "BlobSettings.InnerColor", "BlobSettings.OuterColor",
        "BlobSettings.RadiusInner", "BlobSettings.RadiusOuter"
    };

    GLuint indices[4];
    glGetUniformIndices(programHandle, 4, names, indices);

    GLint offset[4];
    glGetActiveUniformsiv(programHandle, 4, indices, GL_UNIFORM_OFFSET, offset);

    vec4 outerColor = {0.0f, 0.0f, 0.0f, 0.0f};
    vec4 innerColor = {1.0f, 1.0f, 0.75f, 1.0f};
    GLfloat innerRadius = 0.25f, outerRadius = 0.45f;

    memcpy(blockBuffer + offset[0], innerColor, 4 * sizeof(GLfloat));
    memcpy(blockBuffer + offset[1], outerColor, 4 * sizeof(GLfloat));
    memcpy(blockBuffer + offset[2], &innerRadius, sizeof(GLfloat));
    memcpy(blockBuffer + offset[3], &outerRadius, sizeof(GLfloat));

    GLuint uboHandle;
    glGenBuffers(1, &uboHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, blockBuffer, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboHandle);
    free(blockBuffer);
}

void initScene() {
    compile();
    initUniformBlockBuffer();

    float positionData[] = {
        -0.8f, -0.8f, 0.0f,  0.8f, -0.8f, 0.0f,  0.8f,  0.8f, 0.0f,
        -0.8f, -0.8f, 0.0f,  0.8f,  0.8f, 0.0f, -0.8f,  0.8f, 0.0f
    };
    float tcData[] = {
        0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f
    };

    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint tcBufferHandle = vboHandles[1];

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), positionData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, tcBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), tcData, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, tcBufferHandle);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void update(float t) {
    angle += 1.0f;
    if (angle >= 360.0f)
        angle -= 360.0f;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vaoHandle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void resize(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "Uniform Block Example", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW after OpenGL context is created
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Initialize scene
    initScene();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

