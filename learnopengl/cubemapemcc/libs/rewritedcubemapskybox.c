#include <emscripten.h>
#include <emscripten/html5.h>
#include <raylib.h>
#include <GLES3/gl3.h>
#include "gl_wrapper.h"
#include <cglm/cglm.h>
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_SHADER_SIZE 10000
#define MAX_FACES 6

// Function prototypes
void main_loop(void);
GLWTexture LoadTextureGL(const char *path);
GLWTexture LoadCubemapGL(const char* faces[]);
void check_gl_error(const char* operation);

// Global variables
GLWShader shader, skyboxShader;
GLWMesh cubeMesh, skyboxMesh;
GLWTexture cubeTexture, cubemapTexture;
Camera3D camera = { 0 };
bool show_container = true;
float camera_z_offset = 0.0f;
Vector2 lastMousePosition = {0};

int main() {
    // Raylib initialization
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Emscripten with Raylib and GL Wrapper - Skybox");
    if (!IsWindowReady()) {
        printf("Failed to create Raylib window\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    check_gl_error("Enable depth test");

    char* vertex_source = NULL;
    char* fragment_source = NULL;
    GLWrapperError error;

    // Create cubemaps shader
    error = glw_read_file("resources/shaders/cubemap.vs", &vertex_source);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to read cubemaps vertex shader: %s\n", glw_error_string(error));
        return -1;
    }

    error = glw_read_file("resources/shaders/cubemap.fs", &fragment_source);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to read cubemaps fragment shader: %s\n", glw_error_string(error));
        free(vertex_source);
        return -1;
    }

    error = glw_create_shader(vertex_source, fragment_source, &shader);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to create cubemaps shader: %s\n", glw_error_string(error));
        free(vertex_source);
        free(fragment_source);
        return -1;
    }
    free(vertex_source);
    free(fragment_source);

    // Create skybox shader
    error = glw_read_file("resources/shaders/skybox.vs", &vertex_source);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to read skybox vertex shader: %s\n", glw_error_string(error));
        return -1;
    }

    error = glw_read_file("resources/shaders/skybox.fs", &fragment_source);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to read skybox fragment shader: %s\n", glw_error_string(error));
        free(vertex_source);
        return -1;
    }

    error = glw_create_shader(vertex_source, fragment_source, &skyboxShader);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to create skybox shader: %s\n", glw_error_string(error));
        free(vertex_source);
        free(fragment_source);
        return -1;
    }
    free(vertex_source);
    free(fragment_source);

    // Set up vertex data

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

    // Create mesh objects
    error = glw_create_mesh(cubeVertices, sizeof(cubeVertices) / sizeof(float) / 5, NULL, 0, 5 * sizeof(float), &cubeMesh);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to create cube mesh: %s\n", glw_error_string(error));
        return -1;
    }

    error = glw_create_mesh(skyboxVertices, sizeof(skyboxVertices) / sizeof(float) / 3, NULL, 0, 3 * sizeof(float), &skyboxMesh);
    if (error != GL_WRAPPER_SUCCESS) {
        printf("Failed to create skybox mesh: %s\n", glw_error_string(error));
        return -1;
    }

    // Load textures
    cubeTexture = LoadTextureGL("resources/textures/container.png");
    if (cubeTexture.id == 0) {
        printf("Failed to load cube texture\n");
        return -1;
    }

    const char* faces[] = {
        "resources/textures/skybox/right.png",
        "resources/textures/skybox/left.png",
        "resources/textures/skybox/top.png",
        "resources/textures/skybox/bottom.png",
        "resources/textures/skybox/front.png",
        "resources/textures/skybox/back.png"
    };

    cubemapTexture = LoadCubemapGL(faces);
    if (cubemapTexture.id == 0) {
        printf("Failed to load cubemap texture\n");
        return -1;
    }

    printf("Cubemap loaded with ID: %u, dimensions: %dx%d\n", cubemapTexture.id, cubemapTexture.width, cubemapTexture.height);

    // Set up shader uniforms
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTexture.id);
    check_gl_error("Bind cube texture");

    glw_use_shader(&shader);
    GLint texture1Loc = glGetUniformLocation(shader.program, "texture1");
    if (texture1Loc != -1) {
        glUniform1i(texture1Loc, 0);
        printf("Set uniform 'texture1' to 0 in shader program %u\n", shader.program);
    } else {
        printf("Warning: Uniform 'texture1' not found in shader program %u\n", shader.program);
    }
    check_gl_error("Set cube shader uniform");

    glw_use_shader(&skyboxShader);
    GLint skyboxLoc = glGetUniformLocation(skyboxShader.program, "skybox");
    if (skyboxLoc != -1) {
        glUniform1i(skyboxLoc, 0);
        printf("Set uniform 'skybox' to 0 in shader program %u\n", skyboxShader.program);
    } else {
        printf("Warning: Uniform 'skybox' not found in shader program %u\n", skyboxShader.program);
    }
    check_gl_error("Set skybox shader uniform");

    // Initialize camera
    camera.position = (Vector3){ 0.0f, 0.0f, 3.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Set up Emscripten main loop
    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}

void main_loop(void) {
    // Update
    float deltaTime = GetFrameTime();

    // Camera controls
    if (IsKeyDown(KEY_W)) camera.position.z -= 2.5f * deltaTime;
    if (IsKeyDown(KEY_S)) camera.position.z += 2.5f * deltaTime;
    if (IsKeyDown(KEY_A)) camera.position.x -= 2.5f * deltaTime;
    if (IsKeyDown(KEY_D)) camera.position.x += 2.5f * deltaTime;

    // Toggle container visibility
    if (IsKeyPressed(KEY_SPACE)) show_container = !show_container;

    // Change camera Z position
    if (IsKeyDown(KEY_Q)) camera_z_offset += 2.5f * deltaTime;
    if (IsKeyDown(KEY_E)) camera_z_offset -= 2.5f * deltaTime;

    // Mouse look
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 mouseDelta = {mousePosition.x - lastMousePosition.x, mousePosition.y - lastMousePosition.y};
        lastMousePosition = mousePosition;

        camera.target.x += mouseDelta.x * 0.1f;
        camera.target.y -= mouseDelta.y * 0.1f;
    }

    // Draw
    BeginDrawing();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_error("Clear buffers");

    // Set up view and projection matrices
    mat4 view, projection;
    vec3 cameraPos = {camera.position.x, camera.position.y, camera.position.z};
    vec3 cameraTarget = {camera.target.x, camera.target.y, camera.target.z};
    vec3 cameraUp = {camera.up.x, camera.up.y, camera.up.z};
    glm_lookat(cameraPos, cameraTarget, cameraUp, view);

    vec3 offset = {0.0f, 0.0f, camera_z_offset};
    glm_translate(view, offset);

    glm_perspective(glm_rad(camera.fovy), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, projection);
    // After setting view and projection matrices
    printf("View matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("%f %f %f %f\n", view[i][0], view[i][1], view[i][2], view[i][3]);
    }
    printf("Projection matrix:\n");
    for (int i = 0; i < 4; i++) {
        printf("%f %f %f %f\n", projection[i][0], projection[i][1], projection[i][2], projection[i][3]);
    }

    // Draw cube
    if (show_container) {
        glw_use_shader(&shader);
        check_gl_error("Use cube shader");

        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glw_set_uniform_mat4(&shader, "model", model);
        glw_set_uniform_mat4(&shader, "view", view);
        glw_set_uniform_mat4(&shader, "projection", projection);
        check_gl_error("Set cube shader uniforms");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture.id);
        glw_set_uniform_1i(&shader, "texture1", 0);
        check_gl_error("Bind cube texture");

        glBindVertexArray(cubeMesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        check_gl_error("Draw cube");
    }

    // Draw skybox last
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxShader.program);
    check_gl_error("Use skybox shader");

    mat4 skyboxView;
    glm_mat4_copy(view, skyboxView);
    skyboxView[3][0] = skyboxView[3][1] = skyboxView[3][2] = 0.0f; // Remove translation

    GLint viewLoc = glGetUniformLocation(skyboxShader.program, "view");
    GLint projLoc = glGetUniformLocation(skyboxShader.program, "projection");

    if (viewLoc != -1) {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)skyboxView);
    } else {
        printf("Warning: 'view' uniform not found in skybox shader\n");
    }

    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)projection);
    } else {
        printf("Warning: 'projection' uniform not found in skybox shader\n");
    }

    check_gl_error("Set skybox shader uniforms");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture.id);
    check_gl_error("Bind cubemap texture");

    printf("Drawing skybox with VAO: %u\n", skyboxMesh.vao);
    glBindVertexArray(skyboxMesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    check_gl_error("Draw skybox");

    glDepthFunc(GL_LESS);

    // Print debug information
    printf("Camera position: (%f, %f, %f)\n", camera.position.x, camera.position.y, camera.position.z);
    printf("Camera target: (%f, %f, %f)\n", camera.target.x, camera.target.y, camera.target.z);
    printf("Skybox texture ID: %u\n", cubemapTexture.id);

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    printf("Current shader program: %d\n", currentProgram);


    // After drawing the skybox
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    printf("Viewport: %d %d %d %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);

    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    printf("Depth test enabled: %s\n", depthTest ? "true" : "false");

    EndDrawing();

}

GLWTexture LoadTextureGL(const char * path) {
    GLWTexture texture = {0};
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            printf("Error: Unsupported number of components (%d) in texture %s\n", nrComponents, path);
            stbi_image_free(data);
            return texture;
        }

        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texture.width = width;
        texture.height = height;
        texture.format = format;
        texture.target = GL_TEXTURE_2D;

        printf("Texture loaded successfully: %s\n", path);
        printf("Dimensions: %dx%d\n", width, height);
        printf("Components: %d\n", nrComponents);
        printf("Texture ID: %u\n", texture.id);

        stbi_image_free(data);
    } else {
        printf("Texture failed to load at path: %s\n", path);
        printf("stbi_failure_reason: %s\n", stbi_failure_reason());
    }
    return texture;
}

GLWTexture LoadCubemapGL(const char* faces[]) {
    GLWTexture cubemap = {0};
    glGenTextures(1, &cubemap.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            printf("Loaded cubemap face %d: %s (format: %d)\n", i, faces[i], format);
        } else {
            printf("Cubemap texture failed to load at path: %s\n", faces[i]);
            printf("stbi_failure_reason: %s\n", stbi_failure_reason());
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    cubemap.target = GL_TEXTURE_CUBE_MAP;
    cubemap.width = width;
    cubemap.height = height;
    cubemap.format = GL_RGB;
    cubemap.type = GL_UNSIGNED_BYTE;

    printf("Cubemap created with ID: %u\n", cubemap.id);
    return cubemap;
}


void check_gl_error(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL error after %s: 0x%x\n", operation, error);
    }
}
