#include <raylib.h>
#include <emscripten.h>
#include "gl_wrapper.h"

const int screen_width = 800;
const int screen_height = 450;

GLWShader shader;
GLWMesh triangle_mesh;

void draw(void) {
    BeginDrawing();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glw_use_shader(&shader);
    glw_draw_mesh(&triangle_mesh, GL_TRIANGLES);

    EndDrawing();
}

void setup(void) {
    // Vertex shader
    const char* vertex_shader_source = 
        "#version 300 es\n"
        "layout(location = 0) in vec3 position;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "}\n";

    // Fragment shader
    const char* fragment_shader_source = 
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    // Create shader
    GLWrapperError error = glw_create_shader(vertex_shader_source, fragment_shader_source, &shader);
    if (error != GL_WRAPPER_SUCCESS) {
        TraceLog(LOG_ERROR, "Failed to create shader: %s", glw_error_string(error));
        return;
    }

    // Vertex data for a simple triangle
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    // Create mesh
    error = glw_create_mesh(vertices, 3, NULL, 0, 3, &triangle_mesh);
    if (error != GL_WRAPPER_SUCCESS) {
        TraceLog(LOG_ERROR, "Failed to create mesh: %s", glw_error_string(error));
        return;
    }
}

void cleanup(void) {
    glw_delete_shader(&shader);
    glw_delete_mesh(&triangle_mesh);
}

int main() {
    InitWindow(screen_width, screen_height, "GL Wrapper Test - Triangle");

    setup();

    SetTargetFPS(60);

    emscripten_set_main_loop(draw, 0, 1);

    cleanup();
    CloseWindow();

    return 0;
}
