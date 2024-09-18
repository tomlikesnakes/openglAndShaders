#include "raylib.h"
#include "rlgl.h"
#include <emscripten/emscripten.h>

#define GLSL_VERSION 300

// Global variables
Camera camera = { 0 };
Model skybox;

// Function prototypes
void UpdateDrawFrame(void);
void InitGame(void);

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Emscripten Cubemap Skybox");
    
    InitGame();

    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);

    UnloadShader(skybox.materials[0].shader);
    UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
    UnloadModel(skybox);

    CloseWindow();

    return 0;
}

void InitGame(void)
{
    // Initialize camera
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Create skybox
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    skybox = LoadModelFromMesh(cube);

    // Load and compile shaders
    skybox.materials[0].shader = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);

    Image faces[6] = {
        LoadImage("resources/textures/skybox/right.png"),
        LoadImage("resources/textures/skybox/left.png"),
        LoadImage("resources/textures/skybox/top.png"),
        LoadImage("resources/textures/skybox/bottom.png"),
        LoadImage("resources/textures/skybox/front.png"),
        LoadImage("resources/textures/skybox/back.png")
    };

    int faceWidth = faces[0].width;
    int faceHeight = faces[0].height;
    Image verticalImage = GenImageColor(faceWidth, faceHeight * 6, BLANK);

    for (int i = 0; i < 6; i++) {
        ImageDraw(&verticalImage, faces[i], (Rectangle){0, 0, faceWidth, faceHeight}, (Rectangle){0, i * faceHeight, faceWidth, faceHeight}, WHITE);
        UnloadImage(faces[i]);
    }

    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(verticalImage, CUBEMAP_LAYOUT_LINE_VERTICAL);
    UnloadImage(verticalImage);
}

void UpdateDrawFrame(void)
{
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
            DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
            rlEnableBackfaceCulling();
            rlEnableDepthMask();
        EndMode3D();
    EndDrawing();
}
