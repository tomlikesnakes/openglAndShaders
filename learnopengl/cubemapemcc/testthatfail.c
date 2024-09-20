#include <raylib.h>
#include <emscripten.h>
#include <GLES3/gl3.h>  // OpenGL ES 3.0 (WebGL 2)

const int screen_width = 800;
const int screen_height = 450;

GLuint shader_program;
GLint pos_attrib;
GLuint vbo, vao;

float vertices[] = {
    // Vertex positions for a simple triangle
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

void draw(void) {
    BeginDrawing();

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program
    glUseProgram(shader_program);
    TraceLog(LOG_INFO, "Using shader program %d", shader_program);

    // Bind VAO and VBO
    glBindVertexArray(vao);
    TraceLog(LOG_INFO, "Bound VAO %d", vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    TraceLog(LOG_INFO, "Bound VBO %d", vbo);

    // Check if pos_attrib is valid before enabling or using it
    if (pos_attrib >= 0) {
        glEnableVertexAttribArray(pos_attrib);
        TraceLog(LOG_INFO, "Enabled vertex attribute array at location %d", pos_attrib);

        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        TraceLog(LOG_INFO, "VertexAttribPointer set for location %d", pos_attrib);
    } else {
        TraceLog(LOG_ERROR, "Invalid attribute location for 'position'");
    }

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
    TraceLog(LOG_INFO, "DrawArrays executed");

    EndDrawing();
}

void setup_shader_attributes() {
    const char* vertex_shader_source = "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) in vec3 position;\n"
        "void main() {\n"
        "    gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
        "}\n";

    const char* fragment_shader_source = "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glUseProgram(shader_program);

    pos_attrib = glGetAttribLocation(shader_program, "position");
    TraceLog(LOG_INFO, "Attribute 'position' location (queried in setup): %d", pos_attrib);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    if (pos_attrib >= 0) {
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    } else {
        TraceLog(LOG_ERROR, "Invalid attribute location for 'position'");
    }

    glBindVertexArray(0);
}

int main() {
    InitWindow(screen_width, screen_height, "raylib with Emscripten - Simple Draw");

    setup_shader_attributes();

    SetTargetFPS(60);

    emscripten_set_main_loop(draw, 0, 1);

    CloseWindow();

    return 0;
}

