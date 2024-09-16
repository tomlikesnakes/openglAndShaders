#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define numVAOs 1

GLuint renderingProgram;
GLuint vao[numVAOs];

// location of triangle on x axis
float x = 0.0f;
// offset for moving the triangle
float inc = 0.01f;

void printShaderLog(GLuint shader) {
    int len = 0;
    int chWrittn = 0;
    char* log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetShaderInfoLog(shader, len, &chWrittn, log);
        printf("Shader Info Log: %s\n", log);
        free(log);
    }
}

void printProgramLog(int prog) {
    int len = 0;
    int chWrittn = 0;
    char* log;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetProgramInfoLog(prog, len, &chWrittn, log);
        printf("Program Info Log: %s\n", log);
        free(log);
    }
}

int checkOpenGLError() {
    int foundError = 0;
    int glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        printf("glError: %d\n", glErr);
        foundError = 1;
        glErr = glGetError();
    }
    return foundError;
}

char* readShaderSource(const char *filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", filePath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = (char*)malloc(length + 1);
    if (content == NULL) {
        printf("Failed to allocate memory for file content\n");
        fclose(file);
        return NULL;
    }

    fread(content, 1, length, file);
    content[length] = '\0';

    fclose(file);
    return content;
}

GLuint createShaderProgram() {
    GLint vertCompiled;
    GLint fragCompiled;
    GLint linked;
    
    char* vertShaderSrc = readShaderSource("vShader.glsl");
    char* fragShaderSrc = readShaderSource("fShader.glsl");
    
    if (vertShaderSrc == NULL || fragShaderSrc == NULL) {
        return 0;
    }
    
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(vShader, 1, (const GLchar**)&vertShaderSrc, NULL);
    glShaderSource(fShader, 1, (const GLchar**)&fragShaderSrc, NULL);
    
    glCompileShader(vShader);
    checkOpenGLError();
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
    if (vertCompiled != 1) {
        printf("vertex compilation failed\n");
        printShaderLog(vShader);
    }
    
    glCompileShader(fShader);
    checkOpenGLError();
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
    if (fragCompiled != 1) {
        printf("fragment compilation failed\n");
        printShaderLog(fShader);
    }
    
    GLuint vfProgram = glCreateProgram();
    glAttachShader(vfProgram, vShader);
    glAttachShader(vfProgram, fShader);
    
    glLinkProgram(vfProgram);
    checkOpenGLError();
    glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
    if (linked != 1) {
        printf("linking failed\n");
        printProgramLog(vfProgram);
    }
    
    free(vertShaderSrc);
    free(fragShaderSrc);
    
    return vfProgram;
}

void init(GLFWwindow* window) {
    renderingProgram = createShaderProgram();
    glGenVertexArrays(numVAOs, vao);
    glBindVertexArray(vao[0]);
}

void display(GLFWwindow* window, double currentTime) {
    // clear the background to black, each time
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(renderingProgram);
    
    // move the triangle along x axis
    x += inc;
    // switch to moving the triangle to the left
    if (x > 1.0f) inc = -0.01f;
    // switch to moving the triangle to the right
    if (x < -1.0f) inc = 0.01f;
    
    // get ptr to "offset"
    GLuint offsetLoc = glGetUniformLocation(renderingProgram, "offset");
    
    glProgramUniform1f(renderingProgram, offsetLoc, x);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(void) {
	if (!glfwInit()) return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600,600,"Chapter2 - program1", NULL,NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) return 1;
	glfwSwapInterval(1);

	init(window);

	while(!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
