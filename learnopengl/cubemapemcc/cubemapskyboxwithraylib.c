#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #define GLSL_VERSION            300 es
#else
    #define GLSL_VERSION            300 es
#endif

// Global variables for toggles
bool showGrid = true;
bool showFPS = true;
bool showSkybox = true;
bool insideSkybox = false;

// Global variables for camera and movement
Camera camera = { 0 };
Model skybox;
float zoom = 1.0f;
float moveSpeed = 0.1f;

#if defined(PLATFORM_WEB)
// Global variables for key states (web version)
bool keyW = false;
bool keyS = false;
bool keyA = false;
bool keyD = false;
bool keySpace = false;
bool keyLeftControl = false;
bool keyUp = false;
bool keyDown = false;
bool keyTab = false;
bool keyG = false;
bool keyF = false;
bool keyB = false;
bool keyI = false;
bool keyR = false;
#endif

// Function prototypes
void UpdateDrawFrame(void);
void InitGame(void);

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
void UpdateKeyState(int key, bool isDown) {
    switch(key) {
        case 87: keyW = isDown; break;  // W
        case 83: keyS = isDown; break;  // S
        case 65: keyA = isDown; break;  // A
        case 68: keyD = isDown; break;  // D
        case 32: keySpace = isDown; break;  // Space
        case 17: keyLeftControl = isDown; break;  // Left Control
        case 38: keyUp = isDown; break;  // Up Arrow
        case 40: keyDown = isDown; break;  // Down Arrow
        case 9: keyTab = isDown; break;  // Tab
        case 71: keyG = isDown; break;  // G
        case 70: keyF = isDown; break;  // F
        case 66: keyB = isDown; break;  // B
        case 73: keyI = isDown; break;  // I
        case 82: keyR = isDown; break;  // R
    }
}

EMSCRIPTEN_KEEPALIVE
void UpdateMouseWheel(float delta) {
    zoom += delta * 0.1f;
    if (zoom < 0.1f) zoom = 0.1f;
    if (zoom > 3.0f) zoom = 3.0f;
    camera.fovy = 45.0f / zoom;
}
#endif

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - enhanced skybox with inside view");
    
    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
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

    const char* vsFileName = "resources/shaders/skybox.vs";
    const char* fsFileName = "resources/shaders/skybox.fs";

#if defined(PLATFORM_DESKTOP)
    const char* versionDirective = "#version 300 es\n";
#elif defined(PLATFORM_WEB)
    const char* versionDirective = "#version 300 es\n";
#else
    const char* versionDirective = "#version 300 es\n";
#endif

    char* vShaderCode = LoadFileText(vsFileName);
    char* fShaderCode = LoadFileText(fsFileName);

    if (vShaderCode != NULL && fShaderCode != NULL)
    {
        skybox.materials[0].shader = LoadShaderFromMemory(vShaderCode, fShaderCode);

        UnloadFileText(vShaderCode);
        UnloadFileText(fShaderCode);
    }
    else TraceLog(LOG_WARNING, "Failed to load shader files");


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
#if defined(PLATFORM_WEB)
    // Web input handling
    if (keyW) camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), moveSpeed));
    if (keyS) camera.position = Vector3Subtract(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), moveSpeed));
    if (keyA) camera.position = Vector3Subtract(camera.position, Vector3Scale(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position), camera.up)), moveSpeed));
    if (keyD) camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position), camera.up)), moveSpeed));
    if (keySpace) camera.position.y += moveSpeed;
    if (keyLeftControl) camera.position.y -= moveSpeed;

    if (keyUp) moveSpeed *= 1.1f;
    if (keyDown) moveSpeed /= 1.1f;
    if (keyTab) {
        if (IsCursorHidden()) EnableCursor();
        else DisableCursor();
        keyTab = false;
    }
    if (keyG) { showGrid = !showGrid; keyG = false; }
    if (keyF) { showFPS = !showFPS; keyF = false; }
    if (keyB) { showSkybox = !showSkybox; keyB = false; }
    if (keyI) {
        insideSkybox = !insideSkybox;
        if (insideSkybox) camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
        else camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
        keyI = false;
    }
    if (keyR) {
        camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
        camera.target = (Vector3){ 4.0f, 1.0f, 4.0f };
        zoom = 1.0f;
        camera.fovy = 45.0f;
        insideSkybox = false;
        keyR = false;
    }
#else
    // Native input handling
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
        if (insideSkybox) camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
        else camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
    }

    if (IsKeyPressed(KEY_R)) {
        camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };
        camera.target = (Vector3){ 4.0f, 1.0f, 4.0f };
        zoom = 1.0f;
        camera.fovy = 45.0f;
        insideSkybox = false;
    }
#endif

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

            //if (showGrid && !insideSkybox) DrawGrid(10, 1.0f);

        EndMode3D();
    /*

        if (showFPS) DrawFPS(10, 10);

        DrawText("Controls:", 10, 30, 20, BLACK);
        DrawText("WASD: Move | SPACE/CTRL: Up/Down | TAB: Toggle cursor", 10, 50, 10, DARKGRAY);
        DrawText("G: Toggle grid | F: Toggle FPS | B: Toggle skybox", 10, 70, 10, DARKGRAY);
        DrawText("R: Reset | UP/DOWN: Adjust speed | Mouse wheel: Zoom", 10, 90, 10, DARKGRAY);
        DrawText("I: Toggle inside/outside view", 10, 110, 10, DARKGRAY);

        DrawText(TextFormat("Zoom: %.2fx", zoom), 10, 130, 20, BLACK);
        DrawText(TextFormat("View: %s", insideSkybox ? "Inside" : "Outside"), 10, 150, 20, BLACK);
    */

    EndDrawing();
}
