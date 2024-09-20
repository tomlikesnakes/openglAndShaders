// gl_wrapper.h
#ifndef GL_WRAPPER_H
#define GL_WRAPPER_H

#include <GLES3/gl3.h>
#include <cglm/cglm.h>
#include <stdbool.h>

// Error handling
typedef enum {
    GL_WRAPPER_SUCCESS,
    GL_WRAPPER_ERROR_SHADER_COMPILATION,
    GL_WRAPPER_ERROR_SHADER_LINKING,
    GL_WRAPPER_ERROR_TEXTURE_CREATION,
    GL_WRAPPER_ERROR_FRAMEBUFFER_CREATION,
    GL_WRAPPER_ERROR_BUFFER_CREATION,
    GL_WRAPPER_ERROR_FILE_READ,
    GL_WRAPPER_ERROR_MEMORY_ALLOCATION,
    GL_WRAPPER_ERROR_INVALID_ARGUMENT
} GLWrapperError;

// Shader management
typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
} GLWShader;

GLWrapperError glw_create_shader(const char* vertex_source, const char* fragment_source, GLWShader* out_shader);
void glw_delete_shader(GLWShader* shader);
void glw_use_shader(const GLWShader* shader);

// Uniform setters (cached)
GLWrapperError glw_set_uniform_1i(GLWShader* shader, const char* name, int value);
GLWrapperError glw_set_uniform_1f(GLWShader* shader, const char* name, float value);
GLWrapperError glw_set_uniform_vec2(GLWShader* shader, const char* name, vec2 value);
GLWrapperError glw_set_uniform_vec3(GLWShader* shader, const char* name, vec3 value);
GLWrapperError glw_set_uniform_vec4(GLWShader* shader, const char* name, vec4 value);
GLWrapperError glw_set_uniform_mat3(GLWShader* shader, const char* name, mat3 value);
GLWrapperError glw_set_uniform_mat4(GLWShader* shader, const char* name, mat4 value);

// Buffer management
typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int vertex_count;
    int index_count;
    GLenum usage;
} GLWMesh;

GLWrapperError glw_create_mesh(float* vertices, int vertex_count, unsigned int* indices, int index_count, int stride, GLWMesh* out_mesh);
void glw_delete_mesh(GLWMesh* mesh);
void glw_draw_mesh(const GLWMesh* mesh, GLenum draw_mode);
GLWrapperError glw_update_mesh_data(GLWMesh* mesh, float* vertices, int vertex_count, unsigned int* indices, int index_count);

// Texture management
typedef struct {
    GLuint id;
    int width;
    int height;
    GLenum format;
    GLenum internal_format;
    GLenum type;
} GLWTexture;

GLWrapperError glw_create_texture(unsigned char* data, int width, int height, GLenum format, GLenum internal_format, GLenum type, GLWTexture* out_texture);
void glw_delete_texture(GLWTexture* texture);
void glw_bind_texture(const GLWTexture* texture, GLenum texture_unit);

// Framebuffer management
typedef struct {
    GLuint fbo;
    GLWTexture color_texture;
    GLWTexture depth_texture;
    int width;
    int height;
} GLWFramebuffer;

GLWrapperError glw_create_framebuffer(int width, int height, GLWFramebuffer* out_framebuffer);
void glw_delete_framebuffer(GLWFramebuffer* framebuffer);
void glw_bind_framebuffer(const GLWFramebuffer* framebuffer);
void glw_unbind_framebuffer(void);

// Camera
typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
} GLWCamera;

void glw_camera_init(GLWCamera* camera);
void glw_camera_get_view_matrix(const GLWCamera* camera, mat4 view);
void glw_camera_process_keyboard(GLWCamera* camera, int direction, float delta_time);
void glw_camera_process_mouse(GLWCamera* camera, float xoffset, float yoffset);

// Utility functions
const char* glw_error_string(GLWrapperError error);
void glw_check_error(const char* operation);
GLWrapperError glw_read_file(const char* filename, char** out_content);

// Debug functions (can be disabled in release builds)
#ifdef GL_WRAPPER_DEBUG
void glw_log(const char* format, ...);
#else
#define glw_log(...)
#endif

#endif // GL_WRAPPER_H
