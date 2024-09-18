#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>

#define GLSL_VERSION 330

// Initialization of camera
void InitCamera(Camera *camera)
{
    camera->position = (Vector3){ 0.0f, 0.0f, 0.1f };
    camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera->fovy = 90.0f;
    camera->projection = CAMERA_PERSPECTIVE;
}

// Error checking function for shader compilation
void CheckShaderCompilation(Shader shader, const char* shaderType) {
    if (shader.id == 0) {
        printf("ERROR: Failed to compile %s shader\n", shaderType);
        exit(1);
    }
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Debug Cubemap Skybox with Raylib");

    Camera camera = { 0 };
    InitCamera(&camera);

    // Load cubemap images
    Image img[6];
    const char* faceFiles[] = {
        "resources/textures/skybox/right.png",
        "resources/textures/skybox/left.png",
        "resources/textures/skybox/top.png",
        "resources/textures/skybox/bottom.png",
        "resources/textures/skybox/front.png",
        "resources/textures/skybox/back.png"
    };

    for (int i = 0; i < 6; i++) {
        img[i] = LoadImage(faceFiles[i]);
        if (img[i].data == NULL) {
            printf("ERROR: Failed to load image: %s\n", faceFiles[i]);
            exit(1);
        }
        printf("DEBUG: Loaded image %s: %dx%d, format: %d\n", faceFiles[i], img[i].width, img[i].height, img[i].format);
    }

    // Create cubemap
    int size = img[0].width;
    Image verticalStrip = GenImageColor(size, size * 6, BLANK);
    for (int i = 0; i < 6; i++) {
        ImageDraw(&verticalStrip, img[i], (Rectangle){0, 0, size, size}, (Rectangle){0, size * i, size, size}, WHITE);
        UnloadImage(img[i]);
    }

    TextureCubemap cubemap = LoadTextureCubemap(verticalStrip, CUBEMAP_LAYOUT_LINE_VERTICAL);
    UnloadImage(verticalStrip);

    // Debug cubemap info
    printf("DEBUG: Cubemap created - ID: %u, Width: %d, Height: %d, Format: %d, Mipmaps: %d\n", 
           cubemap.id, cubemap.width, cubemap.height, cubemap.format, cubemap.mipmaps);

    if (cubemap.id == 0) {
        printf("ERROR: Failed to create cubemap\n");
        exit(1);
    }

    // Load skybox shader
    Shader shader = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    CheckShaderCompilation(shader, "skybox");

    // Get shader locations
    int viewLoc = GetShaderLocation(shader, "matView");
    int projectionLoc = GetShaderLocation(shader, "matProjection");
    int environmentMapLoc = GetShaderLocation(shader, "environmentMap");

    printf("DEBUG: Shader locations - View: %d, Projection: %d, EnvironmentMap: %d\n", viewLoc, projectionLoc, environmentMapLoc);

    // Set cubemap texture uniform
    SetShaderValueTexture(shader, environmentMapLoc, cubemap);

    // Create skybox mesh (cube)
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube);

    // Set skybox shader
    skybox.materials[0].shader = shader;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                // Draw skybox
                rlDisableBackfaceCulling();
                rlDisableDepthMask();
                
                // Get camera view matrix and remove translation
                Matrix view = GetCameraMatrix(camera);
                view.m12 = 0;
                view.m13 = 0;
                view.m14 = 0;
                
                // Calculate projection matrix
                float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
                Matrix projection = MatrixPerspective(camera.fovy * DEG2RAD, aspect, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
                
                // Set view and projection matrix uniforms
                SetShaderValueMatrix(shader, viewLoc, view);
                SetShaderValueMatrix(shader, projectionLoc, projection);
                
                // Draw skybox
                DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
                
                rlEnableBackfaceCulling();
                rlEnableDepthMask();

                // Draw other 3D objects here
                DrawCube((Vector3){0, 0, 0}, 1.0f, 1.0f, 1.0f, RED);
                DrawCubeWires((Vector3){0, 0, 0}, 1.0f, 1.0f, 1.0f, MAROON);

                DrawGrid(10, 1.0f);
            EndMode3D();

            DrawFPS(10, 10);
            DrawText(TextFormat("Camera: (%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z), 10, 30, 20, BLACK);
            DrawText("Use mouse to look around and WASD to move", 10, 50, 20, BLACK);
        EndDrawing();
    }

    // Unload resources
    UnloadShader(shader);
    UnloadTexture(cubemap);
    UnloadModel(skybox);

    CloseWindow();

    return 0;
}
