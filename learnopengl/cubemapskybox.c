#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_SHADER_SIZE 10000
#define MAX_FACES 6

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(const char *faces[]);

// Camera struct and functions
typedef struct {
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
} Camera;

void camera_init(Camera* camera) {
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, camera->Position);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera->Front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera->Up);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera->WorldUp);  // Make sure this is set
    camera->Yaw = -90.0f;
    camera->Pitch = 0.0f;
    camera->MovementSpeed = 2.5f;
    camera->MouseSensitivity = 0.1f;
    camera->Zoom = 45.0f;

    printf("Camera initialized: Position (%.2f, %.2f, %.2f), Front (%.2f, %.2f, %.2f)\n",
           camera->Position[0], camera->Position[1], camera->Position[2],
           camera->Front[0], camera->Front[1], camera->Front[2]);
}

void camera_get_view_matrix(Camera* camera, mat4 view) {
    vec3 center;
    glm_vec3_add(camera->Position, camera->Front, center);
    glm_lookat(camera->Position, center, camera->Up, view);
}

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool show_container = true;
float camera_z_offset = 0.0f;

// camera
Camera camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Shader struct and functions
typedef struct {
    unsigned int ID;
} Shader;

void shader_init(Shader* shader, const char* vertexPath, const char* fragmentPath) {
    printf("Initializing shader: %s and %s\n", vertexPath, fragmentPath);

    // 1. retrieve the vertex/fragment source code from filePath
    FILE* vShaderFile;
    FILE* fShaderFile;
    char vertexCode[MAX_SHADER_SIZE] = {0};
    char fragmentCode[MAX_SHADER_SIZE] = {0};

    // open files
    vShaderFile = fopen(vertexPath, "rb");
    if (!vShaderFile) {
        fprintf(stderr, "Failed to open vertex shader file: %s\n", vertexPath);
        return;
    }

    fShaderFile = fopen(fragmentPath, "rb");
    if (!fShaderFile) {
        fprintf(stderr, "Failed to open fragment shader file: %s\n", fragmentPath);
        fclose(vShaderFile);
        return;
    }

    printf("Shader files opened successfully\n");

    // read file's buffer contents into strings
    size_t vertexBytesRead = fread(vertexCode, 1, MAX_SHADER_SIZE - 1, vShaderFile);
    size_t fragmentBytesRead = fread(fragmentCode, 1, MAX_SHADER_SIZE - 1, fShaderFile);

    printf("Vertex shader bytes read: %zu\n", vertexBytesRead);
    printf("Fragment shader bytes read: %zu\n", fragmentBytesRead);

    // close file handlers
    fclose(vShaderFile);
    fclose(fShaderFile);

    // Null-terminate the strings
    vertexCode[vertexBytesRead] = '\0';
    fragmentCode[fragmentBytesRead] = '\0';

    printf("Compiling vertex shader...\n");

    // 2. compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    if (vertex == 0) {
        fprintf(stderr, "Failed to create vertex shader\n");
        return;
    }

    const char* vShaderCode = vertexCode;
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        glDeleteShader(vertex);
        return;
    }
    printf("Vertex shader compiled successfully\n");

    printf("Compiling fragment shader...\n");

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragment == 0) {
        fprintf(stderr, "Failed to create fragment shader\n");
        glDeleteShader(vertex);
        return;
    }

    const char* fShaderCode = fragmentCode;
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return;
    }
    printf("Fragment shader compiled successfully\n");

    printf("Linking shader program...\n");

    // shader Program
    shader->ID = glCreateProgram();
    if (shader->ID == 0) {
        fprintf(stderr, "Failed to create shader program\n");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return;
    }

    glAttachShader(shader->ID, vertex);
    glAttachShader(shader->ID, fragment);
    glLinkProgram(shader->ID);

    // print linking errors if any
    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->ID, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(shader->ID);
        shader->ID = 0;
        return;
    }
    printf("Shader program linked successfully\n");

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    printf("Shader initialization complete\n");
}

void shader_use(Shader* shader) {
    glUseProgram(shader->ID);
}

void shader_set_int(Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

void shader_set_mat4(Shader* shader, const char* name, mat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, (float*)mat);
}

// Error checking function
void check_gl_error(const char *file, int line) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char *error;
        switch (err) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                               error = "UNKNOWN"; break;
        }
        printf("GL Error (%s:%d): %s\n", file, line, error);
    }
}

#define CHECK_GL_ERROR() check_gl_error(__FILE__, __LINE__)

int main()
{
    printf("Starting program\n");

    // glfw: initialize and configure
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    printf("GLFW initialized\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    printf("GLFW window created\n");

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        return -1;
    }
    printf("GLEW initialized\n");

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();

    // build and compile shaders
    Shader shader, skyboxShader;
    printf("Compiling shaders...\n");
    shader_init(&shader, "cubemaps.vs", "cubemaps.fs");
    if (shader.ID == 0) {
        fprintf(stderr, "Failed to initialize cubemaps shader\n");
        glfwTerminate();
        return -1;
    }
    shader_init(&skyboxShader, "skybox.vs", "skybox.fs");
    if (skyboxShader.ID == 0) {
        fprintf(stderr, "Failed to initialize skybox shader\n");
        glfwTerminate();
        return -1;
    }
    printf("Shaders compiled successfully\n");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    float skyboxVertices[] = {
        // positions          
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

    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    CHECK_GL_ERROR();
    printf("Cube VAO set up\n");

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    CHECK_GL_ERROR();
    printf("Skybox VAO set up\n");

    // load textures
    printf("Loading cube texture...\n");
    unsigned int cubeTexture = loadTexture("resources/textures/container.jpg");
    if (cubeTexture == 0) {
        fprintf(stderr, "Failed to load cube texture\n");
        return -1;
    }
    printf("Cube texture loaded\n");

    printf("Loading cubemap textures...\n");
    const char* faces[] = {
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    if (cubemapTexture == 0) {
        fprintf(stderr, "Failed to load cubemap textures\n");
        return -1;
    }
    printf("Cubemap textures loaded\n");

    // shader configuration
    shader_use(&shader);
    shader_set_int(&shader, "texture1", 0);
    CHECK_GL_ERROR();

    shader_use(&skyboxShader);
    shader_set_int(&skyboxShader, "skybox", 0);
    CHECK_GL_ERROR();

    // initialize camera
    camera_init(&camera);
    printf("Camera initialized\n");

    // render loop
    printf("Entering render loop\n");
    while (!glfwWindowShouldClose(window))
    {

        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw scene as normal
        shader_use(&shader);
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        mat4 view;
        camera_get_view_matrix(&camera, view);

        // Apply camera Z offset
        vec3 offset = {0.0f, 0.0f, camera_z_offset};
        glm_translate(view, offset);

        mat4 projection;
        glm_perspective(glm_rad(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);
        shader_set_mat4(&shader, "model", model);
        shader_set_mat4(&shader, "view", view);
        shader_set_mat4(&shader, "projection", projection);

        // Print debug information
        printf("  Camera Position: (%.2f, %.2f, %.2f)\n", camera.Position[0], camera.Position[1], camera.Position[2]);
        printf("  Camera Front: (%.2f, %.2f, %.2f)\n", camera.Front[0], camera.Front[1], camera.Front[2]);
        printf("  Camera Up: (%.2f, %.2f, %.2f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);
        printf("  Yaw: %.2f, Pitch: %.2f\n", camera.Yaw, camera.Pitch);


        // cubes
        if (show_container) {
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            printf("Cube rendered\n");
        } else {
            printf("Cube not rendered (hidden)\n");
        }

        CHECK_GL_ERROR();

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        shader_use(&skyboxShader);
        mat4 skyboxView;
        mat3 rotView;
        glm_mat4_copy(view, skyboxView);
        glm_mat4_pick3(skyboxView, rotView);
        glm_mat4_identity(skyboxView);
        glm_mat4_ins3(rotView, skyboxView);
        shader_set_mat4(&skyboxShader, "view", skyboxView);
        shader_set_mat4(&skyboxShader, "projection", projection);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        CHECK_GL_ERROR();
        printf("Skybox rendered\n");

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Print frame time
        printf("Frame time: %.3f ms\n", deltaTime * 1000.0f);
    }
    printf("Exited render loop\n"); 
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);
    CHECK_GL_ERROR();

    glfwTerminate();
    printf("Program terminated successfully\n");
    return 0;
}

// Update the processInput function
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 temp;
        glm_vec3_scale(camera.Front, cameraSpeed, temp);
        glm_vec3_add(camera.Position, temp, camera.Position);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 temp;
        glm_vec3_scale(camera.Front, cameraSpeed, temp);
        glm_vec3_sub(camera.Position, temp, camera.Position);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 temp;
        glm_vec3_cross(camera.Front, camera.Up, temp);
        glm_vec3_normalize(temp);
        glm_vec3_scale(temp, cameraSpeed, temp);
        glm_vec3_sub(camera.Position, temp, camera.Position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 temp;
        glm_vec3_cross(camera.Front, camera.Up, temp);
        glm_vec3_normalize(temp);
        glm_vec3_scale(temp, cameraSpeed, temp);
        glm_vec3_add(camera.Position, temp, camera.Position);
    }

    // Toggle container visibility
    static int space_key_pressed = GLFW_RELEASE;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && space_key_pressed == GLFW_RELEASE) {
        show_container = !show_container;
        space_key_pressed = GLFW_PRESS;
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        space_key_pressed = GLFW_RELEASE;
    }

    // Change camera Z position
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera_z_offset += cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera_z_offset -= cameraSpeed;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.Yaw   += xoffset;
    camera.Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (camera.Pitch > 89.0f)
        camera.Pitch = 89.0f;
    if (camera.Pitch < -89.0f)
        camera.Pitch = -89.0f;

    // update Front, Right and Up Vectors using the updated Euler angles
    vec3 front;
    front[0] = cos(glm_rad(camera.Yaw)) * cos(glm_rad(camera.Pitch));
    front[1] = sin(glm_rad(camera.Pitch));
    front[2] = sin(glm_rad(camera.Yaw)) * cos(glm_rad(camera.Pitch));
    glm_vec3_normalize_to(front, camera.Front);
    // also re-calculate the Right and Up vector
    glm_vec3_cross(camera.Front, camera.WorldUp, camera.Right);
    glm_vec3_normalize(camera.Right);
    glm_vec3_cross(camera.Right, camera.Front, camera.Up);
    glm_vec3_normalize(camera.Up);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.Zoom -= (float)yoffset;
    if (camera.Zoom < 1.0f)
        camera.Zoom = 1.0f;
    if (camera.Zoom > 45.0f)
        camera.Zoom = 45.0f; 
}

// utility function for loading a 2D texture from file
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
{
        printf("Texture failed to load at path: %s\n", path);
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
unsigned int loadCubemap(const char* faces[])
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
    {
            printf("Cubemap texture failed to load at path: %s\n", faces[i]);
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
