#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <stdio.h>


#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #include <GLES3/gl3.h>
    #define GLSL_VERSION            300 es
#else
    #define GLSL_VERSION            300 es
#endif

#define SKYBOX_VERTEX_COUNT 36
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
float rotationSpeed = 0.05f;


// Skybox variables
unsigned int skyboxVBO;
Shader skyboxShader;
unsigned int skyboxVAO;
unsigned int skyboxTexture;

// Player variables
GLuint playerVAO, playerVBO, playerEBO;
Shader playerShader;
Vector3 playerPosition = { 0.0f, 0.0f, 0.0f };
float playerSize = 0.1f;  // Smaller player size

// Function prototypes
void UpdateDrawFrame(void);
void InitGame(void);
void SetupSkybox(void);
void InitializeGrid();
void DrawSkybox(void);
void UnloadSkybox(void);
void LogError(const char* message);
void UpdatePlayerPosition();
void UpdateCameraThirdPerson();
void KeysCamera();
void LogMessage(const char* message);
void InitializePlayerCube();
void DrawPlayerCube();
void DrawDebugCube(Vector3 position, Vector3 size, Color color);
void DrawDebugGrid(int slices, float spacing);
//void CheckGLError(const char* operation);

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



// Add this function to initialize the player cube
void InitializePlayerCube() {
    float vertices[] = {
        // positions          
        -0.25f, -0.25f, -0.25f,
         0.25f, -0.25f, -0.25f,
         0.25f,  0.25f, -0.25f,
        -0.25f,  0.25f, -0.25f,
        -0.25f, -0.25f,  0.25f,
         0.25f, -0.25f,  0.25f,
         0.25f,  0.25f,  0.25f,
        -0.25f,  0.25f,  0.25f
    };
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        1, 5, 6, 6, 2, 1,
        5, 4, 7, 7, 6, 5,
        4, 0, 3, 3, 7, 4,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    glGenVertexArrays(1, &playerVAO);
    glGenBuffers(1, &playerVBO);
    glGenBuffers(1, &playerEBO);

    glBindVertexArray(playerVAO);

    glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, playerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Load and compile player shader
    playerShader = LoadShader("resources/shaders/player.vs", "resources/shaders/player.fs");
}

// Add this function to draw the player cube
void DrawPlayerCube() {
    glUseProgram(playerShader.id);

    Matrix model = MatrixIdentity();
    model = MatrixMultiply(model, MatrixTranslate(playerPosition.x, playerPosition.y, playerPosition.z));

    // Calculate view matrix
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);

    // Calculate projection matrix
    float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
    Matrix projection = MatrixPerspective(camera.fovy*DEG2RAD, aspect, 0.1f, 1000.0f);

    SetShaderValueMatrix(playerShader, GetShaderLocation(playerShader, "model"), model);
    SetShaderValueMatrix(playerShader, GetShaderLocation(playerShader, "view"), view);
    SetShaderValueMatrix(playerShader, GetShaderLocation(playerShader, "projection"), projection);

    // Set color uniform (red in this case)
    float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};  // RGBA for red
    SetShaderValue(playerShader, GetShaderLocation(playerShader, "color"), color, SHADER_UNIFORM_VEC4);

    glBindVertexArray(playerVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - enhanced skybox with inside view");
    
    InitializePlayerCube();
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
        UnloadSkybox();
    UnloadModel(skybox);

    CloseWindow();

    return 0;
}

unsigned int gridVAO = 0;
unsigned int gridVBO = 0;
unsigned int gridColorVBO = 0;
int gridVertexCount = 0;

void InitializeGrid() {
    LogMessage("Initializing grid...");
    const int gridSize = 20; // -10 to 10
    const int lineCount = (gridSize + 1) * 2; // 2 directions
    gridVertexCount = lineCount * 2; // 2 vertices per line
    float* vertices = (float*)malloc(gridVertexCount * 3 * sizeof(float));
    float* colors = (float*)malloc(gridVertexCount * 3 * sizeof(float));

    if (!vertices || !colors) {
        LogError("Failed to allocate memory for grid vertices or colors");
        return;
    }

    int vertexIndex = 0;
    for(int i = -10; i <= 10; i++) {
        float color = (i == 0) ? 0.0f : 0.2f; // Darker color for center lines

        // X-axis line
        vertices[vertexIndex*3] = (float)i;
        vertices[vertexIndex*3+1] = 0.0f;
        vertices[vertexIndex*3+2] = -10.0f;
        colors[vertexIndex*3] = color;
        colors[vertexIndex*3+1] = color;
        colors[vertexIndex*3+2] = color;
        vertexIndex++;

        vertices[vertexIndex*3] = (float)i;
        vertices[vertexIndex*3+1] = 0.0f;
        vertices[vertexIndex*3+2] = 10.0f;
        colors[vertexIndex*3] = color;
        colors[vertexIndex*3+1] = color;
        colors[vertexIndex*3+2] = color;
        vertexIndex++;

        // Z-axis line
        vertices[vertexIndex*3] = -10.0f;
        vertices[vertexIndex*3+1] = 0.0f;
        vertices[vertexIndex*3+2] = (float)i;
        colors[vertexIndex*3] = color;
        colors[vertexIndex*3+1] = color;
        colors[vertexIndex*3+2] = color;
        vertexIndex++;

        vertices[vertexIndex*3] = 10.0f;
        vertices[vertexIndex*3+1] = 0.0f;
        vertices[vertexIndex*3+2] = (float)i;
        colors[vertexIndex*3] = color;
        colors[vertexIndex*3+1] = color;
        colors[vertexIndex*3+2] = color;
        vertexIndex++;
    }

    gridVAO = rlLoadVertexArray();
    rlEnableVertexArray(gridVAO);

    gridVBO = rlLoadVertexBuffer(vertices, gridVertexCount * 3 * sizeof(float), RL_STATIC_DRAW);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(0);

    gridColorVBO = rlLoadVertexBuffer(colors, gridVertexCount * 3 * sizeof(float), RL_STATIC_DRAW);
    rlSetVertexAttribute(1, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(1);

    rlDisableVertexArray();

    free(vertices);
    free(colors);

    LogMessage("Grid initialization complete");
}

void UpdatePlayerPosition() {
    // Update player position based on input
    float speed = 0.1f;
    if (IsKeyDown(KEY_W)) playerPosition.z -= speed;
    if (IsKeyDown(KEY_S)) playerPosition.z += speed;
    if (IsKeyDown(KEY_A)) playerPosition.x -= speed;
    if (IsKeyDown(KEY_D)) playerPosition.x += speed;

    // Keep player within grid bounds
    playerPosition.x = Clamp(playerPosition.x, -10.0f, 10.0f);
    playerPosition.z = Clamp(playerPosition.z, -10.0f, 10.0f);
}
void KeysCamera() {
    // Move camera
    if (IsKeyDown(KEY_W)) camera.position = Vector3Add(camera.position, Vector3Scale(camera.target, moveSpeed));
    if (IsKeyDown(KEY_S)) camera.position = Vector3Subtract(camera.position, Vector3Scale(camera.target, moveSpeed));
    if (IsKeyDown(KEY_A)) camera.position = Vector3Subtract(camera.position, Vector3Scale(Vector3Normalize(Vector3CrossProduct(camera.target, camera.up)), moveSpeed));
    if (IsKeyDown(KEY_D)) camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3CrossProduct(camera.target, camera.up)), moveSpeed));

    // Rotate camera
    if (IsKeyDown(KEY_LEFT)) camera.target = Vector3RotateByAxisAngle(camera.target, camera.up, -rotationSpeed);
    if (IsKeyDown(KEY_RIGHT)) camera.target = Vector3RotateByAxisAngle(camera.target, camera.up, rotationSpeed);
    if (IsKeyDown(KEY_UP)) camera.target = Vector3RotateByAxisAngle(camera.target, Vector3CrossProduct(camera.target, camera.up), -rotationSpeed);
    if (IsKeyDown(KEY_DOWN)) camera.target = Vector3RotateByAxisAngle(camera.target, Vector3CrossProduct(camera.target, camera.up), rotationSpeed);

    // Move player
    if (IsKeyDown(KEY_I)) playerPosition.z -= moveSpeed;
    if (IsKeyDown(KEY_K)) playerPosition.z += moveSpeed;
    if (IsKeyDown(KEY_J)) playerPosition.x -= moveSpeed;
    if (IsKeyDown(KEY_L)) playerPosition.x += moveSpeed;
    if (IsKeyDown(KEY_U)) playerPosition.y += moveSpeed;
    if (IsKeyDown(KEY_O)) playerPosition.y -= moveSpeed;

    // Adjust speeds
    if (IsKeyPressed(KEY_EQUAL)) { moveSpeed *= 1.1f; rotationSpeed *= 1.1f; }
    if (IsKeyPressed(KEY_MINUS)) { moveSpeed /= 1.1f; rotationSpeed /= 1.1f; }
}
void UpdateCameraThirdPerson() {
    // Set camera to follow player from behind and above
    float cameraDistance = 5.0f;
    float cameraHeight = 3.0f;
    camera.position = (Vector3){
        playerPosition.x,
        playerPosition.y + cameraHeight,
        playerPosition.z + cameraDistance
    };
    camera.target = playerPosition;
}

void SetupSkybox(void)
{
    //LogInfo("Setting up skybox...");

    float skyboxSize = 1000.0f;  // Adjust this value as needed
    // Skybox vertices
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
    /*
float skyboxVertices[] = {
    // Front face
    -skyboxSize, -skyboxSize,  skyboxSize,
     skyboxSize, -skyboxSize,  skyboxSize,
     skyboxSize,  skyboxSize,  skyboxSize,
    -skyboxSize,  skyboxSize,  skyboxSize,
    // Back face
    -skyboxSize, -skyboxSize, -skyboxSize,
    -skyboxSize,  skyboxSize, -skyboxSize,
     skyboxSize,  skyboxSize, -skyboxSize,
     skyboxSize, -skyboxSize, -skyboxSize,
    // Top face
    -skyboxSize,  skyboxSize, -skyboxSize,
    -skyboxSize,  skyboxSize,  skyboxSize,
     skyboxSize,  skyboxSize,  skyboxSize,
     skyboxSize,  skyboxSize, -skyboxSize,
    // Bottom face
    -skyboxSize, -skyboxSize, -skyboxSize,
     skyboxSize, -skyboxSize, -skyboxSize,
     skyboxSize, -skyboxSize,  skyboxSize,
    -skyboxSize, -skyboxSize,  skyboxSize,
    // Right face
     skyboxSize, -skyboxSize, -skyboxSize,
     skyboxSize,  skyboxSize, -skyboxSize,
     skyboxSize,  skyboxSize,  skyboxSize,
     skyboxSize, -skyboxSize,  skyboxSize,
    // Left face
    -skyboxSize, -skyboxSize, -skyboxSize,
    -skyboxSize, -skyboxSize,  skyboxSize,
    -skyboxSize,  skyboxSize,  skyboxSize,
    -skyboxSize,  skyboxSize, -skyboxSize
};
*/

    /*
    // Generate and bind VAO with opengl
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    */

    /*
    // Generate and bind VAO with rlgl
    skyboxVAO = rlLoadVertexArray();
    unsigned int vbo = rlLoadVertexBuffer(skyboxVertices, sizeof(skyboxVertices), false);
    rlSetVertexAttribute(0, 3, RL_FLOAT, false, 3*sizeof(float), 0);
    rlEnableVertexAttribute(0);
    */

        // Generate and bind VAO with rlgl
    skyboxVAO = rlLoadVertexArray();
    rlEnableVertexArray(skyboxVAO);

    // Generate and bind VBO
    skyboxVBO = rlLoadVertexBuffer(skyboxVertices, sizeof(skyboxVertices), RL_STATIC_DRAW);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(0);


        rlDisableVertexArray();

    // Load and compile shaders
    skyboxShader = LoadShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");

    // Load cubemap textures
    Image img[6] = {
        LoadImage("resources/textures/skybox/right.png"),
        LoadImage("resources/textures/skybox/left.png"),
        LoadImage("resources/textures/skybox/top.png"),
        LoadImage("resources/textures/skybox/bottom.png"),
        LoadImage("resources/textures/skybox/front.png"),
        LoadImage("resources/textures/skybox/back.png")
    };

    int faceWidth = img[0].width;
    int faceHeight = img[0].height;
    Image verticalImage = GenImageColor(faceWidth, faceHeight * 6, BLANK);

    for (int i = 0; i < 6; i++) {
        ImageDraw(&verticalImage, img[i], (Rectangle){0, 0, faceWidth, faceHeight}, (Rectangle){0, i * faceHeight, faceWidth, faceHeight}, WHITE);
        UnloadImage(img[i]);
    }

    skyboxTexture = LoadTextureCubemap(verticalImage, CUBEMAP_LAYOUT_LINE_VERTICAL).id;
    UnloadImage(verticalImage);

    // Set shader uniforms
    SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "skybox"), (int[1]){0}, SHADER_UNIFORM_INT);


        LogMessage("Skybox setup complete");


    //CheckGLError("SetupSkybox");
    //LogInfo("Skybox setup complete");
}

/* Draw Skybox OPENGL
void DrawSkybox(void)
{
    glDepthFunc(GL_LEQUAL);
    BeginShaderMode(skyboxShader);

    Matrix projection = MatrixPerspective(camera.fovy*DEG2RAD, (double)GetScreenWidth()/(double)GetScreenHeight(), 0.1, 1000.0);
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    view.m0 = 1.0f; view.m1 = 0.0f; view.m2 = 0.0f;
    view.m4 = 0.0f; view.m5 = 1.0f; view.m6 = 0.0f;
    view.m8 = 0.0f; view.m9 = 0.0f; view.m10 = 1.0f;

    SetShaderValueMatrix(skyboxShader, GetShaderLocation(skyboxShader, "projection"), projection);
    SetShaderValueMatrix(skyboxShader, GetShaderLocation(skyboxShader, "view"), view);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    EndShaderMode();
    glDepthFunc(GL_LESS);
}
*/
// Draw Skybox rlgl
/*
void DrawSkybox(void)
{
    rlDisableBackfaceCulling();
    rlDisableDepthMask();

    BeginShaderMode(skyboxShader);

    Matrix projection = MatrixPerspective(camera.fovy*DEG2RAD, (double)GetScreenWidth()/(double)GetScreenHeight(), 0.1, 1000.0);
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    view.m0 = 1.0f; view.m1 = 0.0f; view.m2 = 0.0f;
    view.m4 = 0.0f; view.m5 = 1.0f; view.m6 = 0.0f;
    view.m8 = 0.0f; view.m9 = 0.0f; view.m10 = 1.0f;

    SetShaderValueMatrix(skyboxShader, GetShaderLocation(skyboxShader, "projection"), projection);
    SetShaderValueMatrix(skyboxShader, GetShaderLocation(skyboxShader, "view"), view);

    rlActiveTextureSlot(0);
    rlEnableTextureCubemap(skyboxTexture);

    rlEnableVertexArray(skyboxVAO);
    rlDrawVertexArray(0, 36);
    rlDisableVertexArray();

    rlDisableTextureCubemap();

    EndShaderMode();

    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
*/
void DrawSkybox(void)
{
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    glDepthFunc(GL_LEQUAL);

    BeginShaderMode(skyboxShader);

    Matrix projection = MatrixPerspective(camera.fovy*DEG2RAD, (double)GetScreenWidth()/(double)GetScreenHeight(), 0.1, 10000.0);
    
    // Remove translation from the view matrix
    Matrix view = MatrixLookAt(camera.position, camera.target, camera.up);
    view.m12 = 0;
    view.m13 = 0;
    view.m14 = 0;

    SetShaderValueMatrix(skyboxShader, GetShaderLocation(skyboxShader, "projection"), projection);
    SetShaderValueMatrix(skyboxShader, GetShaderLocation(skyboxShader, "view"), view);

    rlActiveTextureSlot(0);
    rlEnableTextureCubemap(skyboxTexture);

    rlEnableVertexArray(skyboxVAO);
    rlDrawVertexArray(0, 36);  // 6 faces * 2 triangles * 3 vertices
    rlDisableVertexArray();

    rlDisableTextureCubemap();

    EndShaderMode();

    glDepthFunc(GL_LESS);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
/* unload skybox opengl
void UnloadSkybox(void)
{
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    UnloadShader(skyboxShader);
    UnloadTexture(skyboxTexture);
}
*/
void UnloadSkybox(void)
{
    rlUnloadVertexArray(skyboxVAO);
    UnloadShader(skyboxShader);
    rlUnloadTexture(skyboxTexture);
}


void InitGame(void)
{
    // Set up the camera
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };  // Position the camera further back and higher up
    camera.target = (Vector3){ 1.0f, 0.0f, 0.0f };       // Look slightly to the right
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Set initial player position
    playerPosition = (Vector3){ 0.0f, 0.0f, 0.0f };  // Start the player at the center

    InitializeGrid();
    SetupSkybox();
    InitializePlayerCube();

    DisableCursor();

    LogMessage("Game initialized successfully");
}


void UpdateDrawFrame(void)
{
#if defined(PLATFORM_WEB)
    // Web input handling
    if (keyW) playerPosition.z -= moveSpeed;
    if (keyS) playerPosition.z += moveSpeed;
    if (keyA) playerPosition.x -= moveSpeed;
    if (keyD) playerPosition.x += moveSpeed;
    if (keySpace) playerPosition.y += moveSpeed;
    if (keyLeftControl) playerPosition.y -= moveSpeed;

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
        if (insideSkybox) playerPosition = (Vector3){ 0.0f, 0.0f, 0.0f };
        else playerPosition = (Vector3){ 1.0f, 1.0f, 1.0f };
        keyI = false;
    }
    if (keyR) {
        playerPosition = (Vector3){ 1.0f, 1.0f, 1.0f };
        zoom = 1.0f;
        camera.fovy = 45.0f;
        insideSkybox = false;
        keyR = false;
    }
#else
    // Native input handling
    if (IsKeyDown(KEY_W)) playerPosition.z -= moveSpeed;
    if (IsKeyDown(KEY_S)) playerPosition.z += moveSpeed;
    if (IsKeyDown(KEY_A)) playerPosition.x -= moveSpeed;
    if (IsKeyDown(KEY_D)) playerPosition.x += moveSpeed;
    if (IsKeyDown(KEY_SPACE)) playerPosition.y += moveSpeed;
    if (IsKeyDown(KEY_LEFT_CONTROL)) playerPosition.y -= moveSpeed;

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
        if (insideSkybox) playerPosition = (Vector3){ 0.0f, 0.0f, 0.0f };
        else playerPosition = (Vector3){ 1.0f, 1.0f, 1.0f };
    }

    if (IsKeyPressed(KEY_R)) {
        playerPosition = (Vector3){ 1.0f, 1.0f, 1.0f };
        zoom = 1.0f;
        camera.fovy = 45.0f;
        insideSkybox = false;
    }
#endif

    // Keep player within grid bounds
    playerPosition.x = Clamp(playerPosition.x, -10.0f, 10.0f);
    playerPosition.z = Clamp(playerPosition.z, -10.0f, 10.0f);

    // Update camera to follow player
    //UpdateCameraThirdPerson();

    //LogMessage("A");
    //UpdateCamera(&camera, CAMERA_THIRD_PERSON);

    KeysCamera();
    UpdatePlayerPosition();  // Keep your existing player update logic
    //LogMessage("B");
    BeginDrawing();

    //LogMessage("C");
    ClearBackground(RAYWHITE);

    //LogMessage("D");
    BeginMode3D(camera);

    //LogMessage("E");
    if (showSkybox) {
        //LogMessage("F");
        DrawSkybox();
    }

    //LogMessage("O");
    if (showGrid && !insideSkybox) {
        rlEnableShader(rlGetShaderIdDefault());
        rlEnableVertexArray(gridVAO);
        rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
        rlSetVertexAttribute(1, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(0);
        rlEnableVertexAttribute(1);

        rlPushMatrix();
        rlScalef(10.0f, 1.0f, 10.0f);
        rlDrawVertexArray(0, gridVertexCount);
        rlPopMatrix();

        rlDisableVertexArray();
    }

    // Draw player using Raylib's DrawCube function
    //DrawCube(playerPosition, 0.5f, 0.5f, 0.5f, RED);
    //DrawPlayerCube();
    //DrawPlayer();

    //LogMessage("R");
    EndMode3D();

    //LogMessage("S");
    //if (showFPS) DrawFPS(10, 10);

    //LogMessage("S");
    EndDrawing();

    //LogMessage("L");
}

void LogError(const char* message)
{
    TraceLog(LOG_ERROR, message);
    printf("ERROR: %s\n", message);
}

void LogMessage(const char* message) {
    TraceLog(LOG_INFO, "%s", message);
    printf("INFO: %s\n", message);
}
// Add these new functions for debug rendering
void DrawDebugCube(Vector3 position, Vector3 size, Color color)
{
    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlScalef(size.x, size.y, size.z);
        
        rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Front face
        rlVertex3f(-0.5f, -0.5f, 0.5f);  rlVertex3f(0.5f, -0.5f, 0.5f);
        rlVertex3f(0.5f, -0.5f, 0.5f);   rlVertex3f(0.5f, 0.5f, 0.5f);
        rlVertex3f(0.5f, 0.5f, 0.5f);    rlVertex3f(-0.5f, 0.5f, 0.5f);
        rlVertex3f(-0.5f, 0.5f, 0.5f);   rlVertex3f(-0.5f, -0.5f, 0.5f);

        // Back face
        rlVertex3f(-0.5f, -0.5f, -0.5f); rlVertex3f(0.5f, -0.5f, -0.5f);
        rlVertex3f(0.5f, -0.5f, -0.5f);  rlVertex3f(0.5f, 0.5f, -0.5f);
        rlVertex3f(0.5f, 0.5f, -0.5f);   rlVertex3f(-0.5f, 0.5f, -0.5f);
        rlVertex3f(-0.5f, 0.5f, -0.5f);  rlVertex3f(-0.5f, -0.5f, -0.5f);

        // Top face
        rlVertex3f(-0.5f, 0.5f, 0.5f);   rlVertex3f(0.5f, 0.5f, 0.5f);
        rlVertex3f(0.5f, 0.5f, 0.5f);    rlVertex3f(0.5f, 0.5f, -0.5f);
        rlVertex3f(0.5f, 0.5f, -0.5f);   rlVertex3f(-0.5f, 0.5f, -0.5f);
        rlVertex3f(-0.5f, 0.5f, -0.5f);  rlVertex3f(-0.5f, 0.5f, 0.5f);

        // Bottom face
        rlVertex3f(-0.5f, -0.5f, 0.5f);  rlVertex3f(0.5f, -0.5f, 0.5f);
        rlVertex3f(0.5f, -0.5f, 0.5f);   rlVertex3f(0.5f, -0.5f, -0.5f);
        rlVertex3f(0.5f, -0.5f, -0.5f);  rlVertex3f(-0.5f, -0.5f, -0.5f);
        rlVertex3f(-0.5f, -0.5f, -0.5f); rlVertex3f(-0.5f, -0.5f, 0.5f);
        rlEnd();
    rlPopMatrix();
}

void DrawDebugGrid(int slices, float spacing)
{
    int halfSlices = slices / 2;
    
    rlPushMatrix();
        rlBegin(RL_LINES);
        for(int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
            }
            
            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);
            
            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
        rlEnd();
    rlPopMatrix();
}
