#include "raylib.h"
#include "raymath.h"  // Make sure to include this for matrix operations
#include "rlgl.h"
#include <emscripten/emscripten.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include <math.h>

#define GLSL_VERSION 300

// Global variables
Camera camera = { 0 };
Model skybox;
Shader skyboxShader;
unsigned int skyboxVAO = 0;
int frameCount = 0;


// Function prototypes
void UpdateDrawFrame(void);
void InitGame(void);
void LogError(const char* message);
void LogInfo(const char* message);
void CheckGLError(const char* operation);

// Function to create perspective projection matrix
Matrix MatrixPerspectiveCustom(double fovY, double aspect, double nearPlane, double farPlane)
{
    Matrix result = { 0 };
    double f = 1.0 / tan(fovY * 0.5);
    
    result.m0 = f / aspect;
    result.m5 = f;
    result.m10 = (farPlane + nearPlane) / (nearPlane - farPlane);
    result.m11 = -1.0f;
    result.m14 = (2.0 * farPlane * nearPlane) / (nearPlane - farPlane);
    
    return result;
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetTraceLogLevel(LOG_DEBUG);
    LogInfo("Initializing window...");
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Emscripten Cubemap Skybox");
    
    if (!IsWindowReady()) {
        LogError("Failed to create window or OpenGL context");
        return 1;
    }
    LogInfo("Window initialized successfully");

    InitGame();

    LogInfo("Setting up main loop...");
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);

    LogInfo("Cleaning up resources...");
    UnloadShader(skyboxShader);
    UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
    UnloadModel(skybox);

    CloseWindow();
    LogInfo("Application closed successfully");

    return 0;
}

void InitGame(void)
{
    LogInfo("Initializing game...");
    
    // Initialize camera
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    LogInfo("Camera initialized");

    // Create skybox
    LogInfo("Creating skybox mesh...");
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    if (cube.vertexCount == 0) {
        LogError("Failed to generate cube mesh");
        return;
    }
    skybox = LoadModelFromMesh(cube);
    LogInfo("Skybox mesh created successfully");

        // Load and compile shaders
    LogInfo("Loading shaders...");
    skyboxShader = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    if (!IsShaderReady(skyboxShader)) {
        LogError("Failed to load skybox shader");
        return;
    }

    int projectionLoc = GetShaderLocation(skyboxShader, "projection");
    int viewLoc = GetShaderLocation(skyboxShader, "view");
    int skyboxLoc = GetShaderLocation(skyboxShader, "skybox");
    
    if (projectionLoc == -1) LogError("Failed to locate 'projection' uniform");
    if (viewLoc == -1) LogError("Failed to locate 'view' uniform");
    if (skyboxLoc == -1) LogError("Failed to locate 'skybox' uniform");

    // Set the skybox uniform to use texture unit 0
    SetShaderValue(skyboxShader, skyboxLoc, (int[1]){ 0 }, SHADER_UNIFORM_INT);
    
    LogInfo("Shaders loaded successfully");

    LogInfo("Loading skybox textures...");
    Image faces[6];
    const char* faceFiles[] = {
        "resources/textures/skybox/right.png",
        "resources/textures/skybox/left.png",
        "resources/textures/skybox/top.png",
        "resources/textures/skybox/bottom.png",
        "resources/textures/skybox/front.png",
        "resources/textures/skybox/back.png"
    };

    for (int i = 0; i < 6; i++) {
        faces[i] = LoadImage(faceFiles[i]);
        if (!IsImageReady(faces[i])) {
            LogError("Failed to load skybox texture");
            return;
        }
    }

    int faceWidth = faces[0].width;
    int faceHeight = faces[0].height;
    Image verticalImage = GenImageColor(faceWidth, faceHeight * 6, BLANK);
    if (!IsImageReady(verticalImage)) {
        LogError("Failed to generate vertical image for cubemap");
        return;
    }

    for (int i = 0; i < 6; i++) {
        ImageDraw(&verticalImage, faces[i], (Rectangle){0, 0, faceWidth, faceHeight}, (Rectangle){0, i * faceHeight, faceWidth, faceHeight}, WHITE);
        UnloadImage(faces[i]);
    }

    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(verticalImage, CUBEMAP_LAYOUT_LINE_VERTICAL);
    if (!IsTextureReady(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture)) {
        LogError("Failed to load cubemap texture");
        return;
    }
    UnloadImage(verticalImage);
    LogInfo("Skybox textures loaded successfully");

        // Create VAO for skybox using Raylib functions
    LogInfo("Creating VAO for skybox...");
    skyboxVAO = rlLoadVertexArray();
    rlEnableVertexArray(skyboxVAO);

    unsigned int vbo = rlLoadVertexBuffer(skybox.meshes[0].vertices, skybox.meshes[0].vertexCount * 3 * sizeof(float), false);
    rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
    rlEnableVertexAttribute(0);

    rlDisableVertexArray();
    LogInfo("VAO created successfully");

    LogInfo("Game initialization completed");
}

void UpdateDrawFrame(void)
{
    frameCount++;
    if (frameCount % 60 == 0) {
        LogInfo(TextFormat("Frame %d", frameCount));
    }

    LogInfo("A");
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    LogInfo("B");

    BeginMode3D(camera);
    
    LogInfo("C");
    // Use Raylib's functions instead of direct OpenGL calls
    rlDisableBackfaceCulling();
    rlDisableDepthTest();
    
    LogInfo("D");
    // Use the skybox shader
    rlEnableShader(skyboxShader.id);
    
    // Bind the cubemap texture
    rlActiveTextureSlot(0);
    rlEnableTextureCubemap(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture.id);
    
    LogInfo("E");
    // Set uniforms
    float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
    Matrix projection = MatrixPerspective(camera.fovy * DEG2RAD, aspect, 0.01f, 1000.0f);
    Matrix view = GetCameraMatrix(camera);
    view.m12 = 0.0f; view.m13 = 0.0f; view.m14 = 0.0f; // Remove translation

    rlSetUniformMatrix(GetShaderLocation(skyboxShader, "projection"), projection);
    rlSetUniformMatrix(GetShaderLocation(skyboxShader, "view"), view);
    rlSetUniform(GetShaderLocation(skyboxShader, "skybox"), (int[1]){0}, SHADER_UNIFORM_INT, 1);
    
    LogInfo("F");
    // Draw the skybox
    rlEnableVertexArray(skyboxVAO);
    rlDrawVertexArray(0, 36);
    rlDisableVertexArray();
    
    LogInfo("G");
    // Disable the shader and texture
    rlDisableShader();
    rlDisableTextureCubemap();
    
    // Re-enable depth test and backface culling
    rlEnableDepthTest();
    rlEnableBackfaceCulling();
    
    LogInfo("H");
    EndMode3D();
    EndDrawing();
}

void LogError(const char* message)
{
    TraceLog(LOG_ERROR, message);
    printf("ERROR: %s\n", message);
}

void LogInfo(const char* message)
{
    TraceLog(LOG_INFO, message);
    printf("INFO: %s\n", message);
}

void CheckGLError(const char* operation)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        const char* errorString;
        switch (error)
        {
            case GL_INVALID_ENUM:                  errorString = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 errorString = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             errorString = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 errorString = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorString = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                               errorString = "UNKNOWN"; break;
        }
        LogError(TextFormat("OpenGL error after %s: %s", operation, errorString));
    }
}
