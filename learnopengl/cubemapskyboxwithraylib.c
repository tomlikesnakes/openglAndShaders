#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
    #define GLSL_VERSION            100
#else
    #define GLSL_VERSION            330
#endif

// Global variables for toggles
bool showGrid = true;
bool showFPS = true;
bool showSkybox = true;
bool insideSkybox = false;

// Global variables for Emscripten
Camera camera = { 0 };
Model skybox;
float zoom = 1.0f;
float moveSpeed = 0.1f;

// Function prototypes
void UpdateDrawFrame(void);
void InitGame(void);

#ifdef __EMSCRIPTEN__
void emscripten_loop(void)
{
    UpdateDrawFrame();
}
#endif

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - enhanced skybox with inside view");
    
    InitGame();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadShader(skybox.materials[0].shader);
    UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
    UnloadModel(skybox);

    CloseWindow();

    return 0;
}

void InitGame(void)
{
    camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
    camera.target = (Vector3){ 4.0f, 1.0f, 4.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    skybox = LoadModelFromMesh(cube);

    skybox.materials[0].shader = LoadShader(TextFormat("resources/shaders/skybox.vs", GLSL_VERSION),
                                            TextFormat("resources/shaders/skybox.fs", GLSL_VERSION));

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

    DisableCursor();
}

void UpdateDrawFrame(void)
{
    if (IsKeyDown(KEY_W)) camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), moveSpeed));
    if (IsKeyDown(KEY_S)) camera.position = Vector3Subtract(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), moveSpeed));
    if (IsKeyDown(KEY_A)) camera.position = Vector3Subtract(camera.position, Vector3Scale(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position), camera.up)), moveSpeed));
    if (IsKeyDown(KEY_D)) camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position), camera.up)), moveSpeed));
    if (IsKeyDown(KEY_SPACE)) camera.position.y += moveSpeed;
    if (IsKeyDown(KEY_LEFT_CONTROL)) camera.position.y -= moveSpeed;

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        zoom += wheel * 0.1f;
        if (zoom < 0.1f) zoom = 0.1f;
        if (zoom > 3.0f) zoom = 3.0f;
        camera.fovy = 45.0f / zoom;
    }

    if (IsKeyPressed(KEY_UP)) moveSpeed *= 1.1f;
    if (IsKeyPressed(KEY_DOWN)) moveSpeed /= 1.1f;
    if (IsKeyPressed(KEY_TAB)) {
        if (IsCursorHidden()) EnableCursor();
        else DisableCursor();
    }
    if (IsKeyPressed(KEY_G)) showGrid = !showGrid;
    if (IsKeyPressed(KEY_F)) showFPS = !showFPS;
    if (IsKeyPressed(KEY_B)) showSkybox = !showSkybox;

    if (IsKeyPressed(KEY_I)) {
        insideSkybox = !insideSkybox;
        if (insideSkybox) {
            camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
        } else {
            camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
        }
    }

    if (IsKeyPressed(KEY_R)) {
        camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
        camera.target = (Vector3){ 4.0f, 1.0f, 4.0f };
        zoom = 1.0f;
        camera.fovy = 45.0f;
        insideSkybox = false;
    }

    UpdateCamera(&camera, CAMERA_FIRST_PERSON);

    BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

            if (showSkybox) {
                rlDisableBackfaceCulling();
                rlDisableDepthMask();
                if (insideSkybox) {
                    rlScalef(500.0f, 500.0f, 500.0f);
                }
                DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
                rlEnableBackfaceCulling();
                rlEnableDepthMask();
            }

            if (showGrid && !insideSkybox) DrawGrid(10, 1.0f);

        EndMode3D();

        if (showFPS) DrawFPS(10, 10);

        DrawText("Controls:", 10, 30, 20, BLACK);
        DrawText("WASD: Move | SPACE/CTRL: Up/Down | TAB: Toggle cursor", 10, 50, 10, DARKGRAY);
        DrawText("G: Toggle grid | F: Toggle FPS | B: Toggle skybox", 10, 70, 10, DARKGRAY);
        DrawText("R: Reset | UP/DOWN: Adjust speed | Mouse wheel: Zoom", 10, 90, 10, DARKGRAY);
        DrawText("I: Toggle inside/outside view", 10, 110, 10, DARKGRAY);

        DrawText(TextFormat("Zoom: %.2fx", zoom), 10, 130, 20, BLACK);
        DrawText(TextFormat("View: %s", insideSkybox ? "Inside" : "Outside"), 10, 150, 20, BLACK);

    EndDrawing();
}
