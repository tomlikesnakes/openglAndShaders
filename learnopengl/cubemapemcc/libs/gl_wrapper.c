#include "gl_wrapper.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_SHADER_LOG_SIZE 512
#define MAX_UNIFORM_NAME 64
#define UNIFORM_CACHE_SIZE 128

typedef struct {
    char name[MAX_UNIFORM_NAME];
    GLint location;
} UniformLocation;

typedef struct {
    UniformLocation locations[UNIFORM_CACHE_SIZE];
    int count;
} UniformCache;

static UniformCache uniform_cache = {0};

// Shader compilation and linking
static GLWrapperError compile_shader(GLuint shader, const char* source) {
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[MAX_SHADER_LOG_SIZE];
        glGetShaderInfoLog(shader, MAX_SHADER_LOG_SIZE, NULL, info_log);
        glw_log("ERROR::SHADER::COMPILATION_FAILED\n%s\n", info_log);
        return GL_WRAPPER_ERROR_SHADER_COMPILATION;
    }
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_create_shader(const char* vertex_source, const char* fragment_source, GLWShader* out_shader) {
    GLWrapperError error = GL_WRAPPER_SUCCESS;
    out_shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    out_shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    error = compile_shader(out_shader->vertex_shader, vertex_source);
    if (error != GL_WRAPPER_SUCCESS) return error;

    error = compile_shader(out_shader->fragment_shader, fragment_source);
    if (error != GL_WRAPPER_SUCCESS) return error;

    out_shader->program = glCreateProgram();
    glAttachShader(out_shader->program, out_shader->vertex_shader);
    glAttachShader(out_shader->program, out_shader->fragment_shader);
    glLinkProgram(out_shader->program);

    GLint success;
    glGetProgramiv(out_shader->program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[MAX_SHADER_LOG_SIZE];
        glGetProgramInfoLog(out_shader->program, MAX_SHADER_LOG_SIZE, NULL, info_log);
        glw_log("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
        return GL_WRAPPER_ERROR_SHADER_LINKING;
    }

    return GL_WRAPPER_SUCCESS;
}

void glw_delete_shader(GLWShader* shader) {
    glDeleteShader(shader->vertex_shader);
    glDeleteShader(shader->fragment_shader);
    glDeleteProgram(shader->program);
    *shader = (GLWShader){0};
}

void glw_use_shader(const GLWShader* shader) {
    glUseProgram(shader->program);
}

// Uniform handling
static GLint get_uniform_location(GLWShader* shader, const char* name) {
    for (int i = 0; i < uniform_cache.count; i++) {
        if (strcmp(uniform_cache.locations[i].name, name) == 0) {
            return uniform_cache.locations[i].location;
        }
    }

    GLint location = glGetUniformLocation(shader->program, name);
    if (location == -1) {
        glw_log("Warning: Uniform '%s' not found in shader program %u\n", name, shader->program);
    } else if (uniform_cache.count < UNIFORM_CACHE_SIZE) {
        strncpy(uniform_cache.locations[uniform_cache.count].name, name, MAX_UNIFORM_NAME - 1);
        uniform_cache.locations[uniform_cache.count].location = location;
        uniform_cache.count++;
    }

    return location;
}

GLWrapperError glw_set_uniform_1i(GLWShader* shader, const char* name, int value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniform1i(location, value);
    glw_check_error("glw_set_uniform_1i");
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_set_uniform_1f(GLWShader* shader, const char* name, float value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniform1f(location, value);
    glw_check_error("glw_set_uniform_1f");
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_set_uniform_vec2(GLWShader* shader, const char* name, vec2 value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniform2fv(location, 1, value);
    glw_check_error("glw_set_uniform_vec2");
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_set_uniform_vec3(GLWShader* shader, const char* name, vec3 value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniform3fv(location, 1, value);
    glw_check_error("glw_set_uniform_vec3");
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_set_uniform_vec4(GLWShader* shader, const char* name, vec4 value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniform4fv(location, 1, value);
    glw_check_error("glw_set_uniform_vec4");
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_set_uniform_mat3(GLWShader* shader, const char* name, mat3 value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniformMatrix3fv(location, 1, GL_FALSE, (float*)value);
    glw_check_error("glw_set_uniform_mat3");
    return GL_WRAPPER_SUCCESS;
}

GLWrapperError glw_set_uniform_mat4(GLWShader* shader, const char* name, mat4 value) {
    GLint location = get_uniform_location(shader, name);
    if (location == -1) return GL_WRAPPER_ERROR_SHADER_LINKING;
    glUseProgram(shader->program);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)value);
    glw_check_error("glw_set_uniform_mat4");
    return GL_WRAPPER_SUCCESS;
}

// Mesh management
GLWrapperError glw_create_mesh(float* vertices, int vertex_count, unsigned int* indices, int index_count, int stride, GLWMesh* out_mesh) {
    out_mesh->vertex_count = vertex_count;
    out_mesh->index_count = index_count;

    glGenVertexArrays(1, &out_mesh->vao);
    glGenBuffers(1, &out_mesh->vbo);

    glBindVertexArray(out_mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, out_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * stride, vertices, GL_STATIC_DRAW);

    if (indices && index_count > 0) {
        glGenBuffers(1, &out_mesh->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    }

    // Set up vertex attributes based on stride
    if (stride == 5 * sizeof(float)) {  // For cube (position + texture coordinates)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    } else if (stride == 3 * sizeof(float)) {  // For skybox (position only)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);
    }

    printf("Creating mesh with %d vertices, stride: %d\n", vertex_count, stride);
    printf("VAO: %u, VBO: %u\n", out_mesh->vao, out_mesh->vbo);

    // Print the first few vertices
    printf("First few vertices:\n");
    for (int i = 0; i < 6 && i < vertex_count; i++) {
        printf("%f %f %f\n", vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
    }

    glBindVertexArray(0);

    return GL_WRAPPER_SUCCESS;
}

void glw_delete_mesh(GLWMesh* mesh) {
    if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
    if (mesh->vbo) glDeleteBuffers(1, &mesh->vbo);
    if (mesh->ebo) glDeleteBuffers(1, &mesh->ebo);
    *mesh = (GLWMesh){0};
}

void glw_draw_mesh(const GLWMesh* mesh, GLenum draw_mode) {
    glBindVertexArray(mesh->vao);
    if (mesh->index_count > 0) {
        glDrawElements(draw_mode, mesh->index_count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(draw_mode, 0, mesh->vertex_count);
    }
    glBindVertexArray(0);
}

GLWrapperError glw_update_mesh_data(GLWMesh* mesh, float* vertices, int vertex_count, unsigned int* indices, int index_count) {
    if (vertex_count > mesh->vertex_count || index_count > mesh->index_count) {
        return GL_WRAPPER_ERROR_INVALID_ARGUMENT;
    }

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(float), vertices);

    if (indices && mesh->ebo) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_count * sizeof(unsigned int), indices);
    }

    glBindVertexArray(0);

    return GL_WRAPPER_SUCCESS;
}

// Texture management
GLWrapperError glw_create_texture(unsigned char* data, int width, int height, GLenum format, GLenum internal_format, GLenum type, GLWTexture* out_texture) {
    out_texture->width = width;
    out_texture->height = height;
    out_texture->format = format;
    out_texture->internal_format = internal_format;
    out_texture->type = type;

    glGenTextures(1, &out_texture->id);
    glBindTexture(GL_TEXTURE_2D, out_texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return GL_WRAPPER_SUCCESS;
}

void glw_delete_texture(GLWTexture* texture) {
    glDeleteTextures(1, &texture->id);
    *texture = (GLWTexture){0};
}

void glw_bind_texture(const GLWTexture* texture, GLenum texture_unit) {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(texture->target, texture->id);
}


// Framebuffer management
GLWrapperError glw_create_framebuffer(int width, int height, GLWFramebuffer* out_framebuffer) {
    out_framebuffer->width = width;
    out_framebuffer->height = height;

    glGenFramebuffers(1, &out_framebuffer->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, out_framebuffer->fbo);

    // Color attachment
    unsigned char* empty_data = calloc(width * height * 4, sizeof(unsigned char));
    GLWrapperError error = glw_create_texture(empty_data, width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &out_framebuffer->color_texture);
    free(empty_data);
    if (error != GL_WRAPPER_SUCCESS) return error;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out_framebuffer->color_texture.id, 0);

    // Depth attachment
    error = glw_create_texture(NULL, width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_UNSIGNED_INT, &out_framebuffer->depth_texture);
    if (error != GL_WRAPPER_SUCCESS) return error;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, out_framebuffer->depth_texture.id, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glw_log("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        return GL_WRAPPER_ERROR_FRAMEBUFFER_CREATION;}

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return GL_WRAPPER_SUCCESS;
}

void glw_delete_framebuffer(GLWFramebuffer* framebuffer) {
    glDeleteFramebuffers(1, &framebuffer->fbo);
    glw_delete_texture(&framebuffer->color_texture);
    glw_delete_texture(&framebuffer->depth_texture);
    *framebuffer = (GLWFramebuffer){0};
}

void glw_bind_framebuffer(const GLWFramebuffer* framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->fbo);
    glViewport(0, 0, framebuffer->width, framebuffer->height);
}

void glw_unbind_framebuffer(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Camera
void glw_camera_init(GLWCamera* camera) {
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, camera->position);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera->front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera->up);
    camera->yaw = -90.0f;
    camera->pitch = 0.0f;
}

void glw_camera_get_view_matrix(const GLWCamera* camera, mat4 view) {
    vec3 position, front, up, center;
    glm_vec3_copy(camera->position, position);
    glm_vec3_copy(camera->front, front);
    glm_vec3_copy(camera->up, up);

    glm_vec3_add(position, front, center);
    glm_lookat(position, center, up, view);
}

void glw_camera_process_keyboard(GLWCamera* camera, int direction, float delta_time) {
    float velocity = 2.5f * delta_time;
    vec3 temp, front, up;
    glm_vec3_copy(camera->front, front);
    glm_vec3_copy(camera->up, up);

    switch(direction) {
        case 0: // FORWARD
            glm_vec3_scale(front, velocity, temp);
            glm_vec3_add(camera->position, temp, camera->position);
            break;
        case 1: // BACKWARD
            glm_vec3_scale(front, velocity, temp);
            glm_vec3_sub(camera->position, temp, camera->position);
            break;
        case 2: // LEFT
            glm_vec3_cross(front, up, temp);
            glm_normalize(temp);
            glm_vec3_scale(temp, velocity, temp);
            glm_vec3_sub(camera->position, temp, camera->position);
            break;
        case 3: // RIGHT
            glm_vec3_cross(front, up, temp);
            glm_normalize(temp);
            glm_vec3_scale(temp, velocity, temp);
            glm_vec3_add(camera->position, temp, camera->position);
            break;
    }
}

void glw_camera_process_mouse(GLWCamera* camera, float xoffset, float yoffset) {
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    vec3 front;
    front[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front[1] = sin(glm_rad(camera->pitch));
    front[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    glm_normalize_to(front, camera->front);
}

// Utility functions
const char* glw_error_string(GLWrapperError error) {
    switch (error) {
        case GL_WRAPPER_SUCCESS: return "Success";
        case GL_WRAPPER_ERROR_SHADER_COMPILATION: return "Shader compilation error";
        case GL_WRAPPER_ERROR_SHADER_LINKING: return "Shader linking error";
        case GL_WRAPPER_ERROR_TEXTURE_CREATION: return "Texture creation error";
        case GL_WRAPPER_ERROR_FRAMEBUFFER_CREATION: return "Framebuffer creation error";
        case GL_WRAPPER_ERROR_BUFFER_CREATION: return "Buffer creation error";
        case GL_WRAPPER_ERROR_FILE_READ: return "File read error";
        case GL_WRAPPER_ERROR_MEMORY_ALLOCATION: return "Memory allocation error";
        case GL_WRAPPER_ERROR_INVALID_ARGUMENT: return "Invalid argument error";
        default: return "Unknown error";
    }
}

void glw_check_error(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        glw_log("OpenGL error after %s: 0x%x\n", operation, error);
    }
}

GLWrapperError glw_read_file(const char* filename, char** out_content) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        glw_log("Failed to open file: %s\nError: %s\n", filename, strerror(errno));
        return GL_WRAPPER_ERROR_FILE_READ;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        glw_log("Failed to seek to end of file: %s\nError: %s\n", filename, strerror(errno));
        fclose(file);
        return GL_WRAPPER_ERROR_FILE_READ;
    }

    long length = ftell(file);
    if (length == -1) {
        glw_log("Failed to get file size: %s\nError: %s\n", filename, strerror(errno));
        fclose(file);
        return GL_WRAPPER_ERROR_FILE_READ;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        glw_log("Failed to seek to start of file: %s\nError: %s\n", filename, strerror(errno));
        fclose(file);
        return GL_WRAPPER_ERROR_FILE_READ;
    }

    *out_content = (char*)malloc(length + 1);
    if (!*out_content) {
        glw_log("Failed to allocate memory for file content: %s\nError: %s\n", filename, strerror(errno));
        fclose(file);
        return GL_WRAPPER_ERROR_MEMORY_ALLOCATION;
    }

    size_t read_length = fread(*out_content, 1, length, file);
    if (read_length != length) {
        glw_log("Failed to read entire file: %s\nExpected %ld bytes, read %zu bytes\nError: %s\n",
                filename, length, read_length, ferror(file) ? strerror(errno) : "Unknown error");
        free(*out_content);
        fclose(file);
        return GL_WRAPPER_ERROR_FILE_READ;
    }

    (*out_content)[length] = '\0';
    fclose(file);
    return GL_WRAPPER_SUCCESS;
}
/*
GLWrapperError glw_read_file(const char* filename, char** out_content) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        glw_log("Failed to open file: %s\n", filename);
        return GL_WRAPPER_ERROR_FILE_READ;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *out_content = (char*)malloc(length + 1);
    if (!*out_content) {
        fclose(file);
        return GL_WRAPPER_ERROR_MEMORY_ALLOCATION;
    }

    fread(*out_content, 1, length, file);
    (*out_content)[length] = '\0';

    fclose(file);
    return GL_WRAPPER_SUCCESS;
}
*/

#ifdef GL_WRAPPER_DEBUG
void glw_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
#endif // GL_WRAPPER_DEBUG
