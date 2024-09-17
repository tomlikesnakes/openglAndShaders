#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(USE_CGLM) && !defined(USE_RAYMATH)
#include <math.h>
#endif

// File operations
char* read_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", file_path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* content = (char*)malloc(file_size + 1);
    if (content == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    fread(content, 1, file_size, file);
    content[file_size] = '\0';

    fclose(file);
    return content;
}

// OpenGL-specific functions
#ifdef USE_OPENGL
#include <GL/glew.h>

int check_opengl_error(void) {
    int found_error = 0;
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("glError: %d\n", error);
        found_error = 1;
    }
    return found_error;
}

void print_shader_log(unsigned int shader) {
    int len = 0;
    int chars_written = 0;
    char* log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetShaderInfoLog(shader, len, &chars_written, log);
        printf("Shader Info Log: %s\n", log);
        free(log);
    }
}

void print_program_log(int prog) {
    int len = 0;
    int chars_written = 0;
    char* log;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetProgramInfoLog(prog, len, &chars_written, log);
        printf("Program Info Log: %s\n", log);
        free(log);
    }
}

unsigned int prepare_shader(int shader_type, const char* shader_path) {
    unsigned int shader_ref;
    int shader_compiled;
    
    char* shader_str = read_file(shader_path);
    if (shader_str == NULL) {
        return 0;
    }
    
    shader_ref = glCreateShader(shader_type);
    const char* shader_src = shader_str;
    glShaderSource(shader_ref, 1, &shader_src, NULL);
    glCompileShader(shader_ref);
    check_opengl_error();
    
    glGetShaderiv(shader_ref, GL_COMPILE_STATUS, &shader_compiled);
    if (shader_compiled != 1) {
        printf("Shader compilation error.\n");
        print_shader_log(shader_ref);
    }
    
    free(shader_str);
    return shader_ref;
}

int finalize_shader_program(unsigned int program) {
    int linked;
    glLinkProgram(program);
    check_opengl_error();
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != 1) {
        printf("Linking failed\n");
        print_program_log(program);
    }
    return program;
}

unsigned int create_shader_program(const char* vp, const char* fp) {
    unsigned int v_shader = prepare_shader(GL_VERTEX_SHADER, vp);
    unsigned int f_shader = prepare_shader(GL_FRAGMENT_SHADER, fp);
    unsigned int vf_program = glCreateProgram();
    glAttachShader(vf_program, v_shader);
    glAttachShader(vf_program, f_shader);
    return finalize_shader_program(vf_program);
}

unsigned int create_shader_program_with_geometry(const char* vp, const char* gp, const char* fp) {
    unsigned int v_shader = prepare_shader(GL_VERTEX_SHADER, vp);
    unsigned int g_shader = prepare_shader(GL_GEOMETRY_SHADER, gp);
    unsigned int f_shader = prepare_shader(GL_FRAGMENT_SHADER, fp);
    unsigned int vgf_program = glCreateProgram();
    glAttachShader(vgf_program, v_shader);
    glAttachShader(vgf_program, g_shader);
    glAttachShader(vgf_program, f_shader);
    return finalize_shader_program(vgf_program);
}
#endif // USE_OPENGL

// Math functions
#if !defined(USE_CGLM) && !defined(USE_RAYMATH)
void matrix_multiply(float* m1, float* m2, float* result) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[i * 4 + j] += m1[i * 4 + k] * m2[k * 4 + j];
            }
        }
    }
}

void matrix_translate(float* m, float x, float y, float z) {
    float translation[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x,    y,    z,    1.0f
    };
    float result[16];
    matrix_multiply(m, translation, result);
    memcpy(m, result, 16 * sizeof(float));
}

void matrix_rotate(float* m, float angle, float x, float y, float z) {
    float radians = angle * 3.14159f / 180.0f;
    float c = cosf(radians);
    float s = sinf(radians);
    float magnitude = sqrtf(x*x + y*y + z*z);
    float nx = x / magnitude;
    float ny = y / magnitude;
    float nz = z / magnitude;
    float rotation[16] = {
        nx*nx*(1-c)+c,    ny*nx*(1-c)+nz*s, nz*nx*(1-c)-ny*s, 0.0f,
        nx*ny*(1-c)-nz*s, ny*ny*(1-c)+c,    nz*ny*(1-c)+nx*s, 0.0f,
        nx*nz*(1-c)+ny*s, ny*nz*(1-c)-nx*s, nz*nz*(1-c)+c,    0.0f,
        0.0f,             0.0f,             0.0f,             1.0f
    };
    float result[16];
    matrix_multiply(m, rotation, result);
    memcpy(m, result, 16 * sizeof(float));
}

void matrix_scale(float* m, float x, float y, float z) {
    float scale[16] = {
        x,    0.0f, 0.0f, 0.0f,
        0.0f, y,    0.0f, 0.0f,
        0.0f, 0.0f, z,    0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    float result[16];
    matrix_multiply(m, scale, result);
    memcpy(m, result, 16 * sizeof(float));
}

void matrix_perspective(float* m, float fovy, float aspect, float near, float far) {
    float f = 1.0f / tanf(fovy / 2.0f);
    float nf = 1.0f / (near - far);

    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (far + near) * nf;
    m[11] = -1.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 2.0f * far * near * nf;
    m[15] = 0.0f;
}

void matrix_identity(float* m) {
    m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f; m[12] = 0.0f;
    m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f; m[13] = 0.0f;
    m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}
#endif // !defined(USE_CGLM) && !defined(USE_RAYMATH)

// Wrapper functions for matrix operations
void utils_matrix_multiply(float* m1, float* m2, float* result) {
#ifdef USE_CGLM
    glm_mat4_mul((vec4*)m1, (vec4*)m2, (vec4*)result);
#elif defined(USE_RAYMATH)
    *((Matrix*)result) = MatrixMultiply(*((Matrix*)m1), *((Matrix*)m2));
#else
    matrix_multiply(m1, m2, result);
#endif
}

void utils_matrix_translate(float* m, float x, float y, float z) {
#ifdef USE_CGLM
    glm_translate((vec4*)m, (vec3){x, y, z});
#elif defined(USE_RAYMATH)
    *((Matrix*)m) = MatrixMultiply(*((Matrix*)m), MatrixTranslate(x, y, z));
#else
    matrix_translate(m, x, y, z);
#endif
}

void utils_matrix_rotate(float* m, float angle, float x, float y, float z) {
#ifdef USE_CGLM
    glm_rotate((vec4*)m, angle, (vec3){x, y, z});
#elif defined(USE_RAYMATH)
    *((Matrix*)m) = MatrixMultiply(*((Matrix*)m), MatrixRotate((Vector3){x, y, z}, angle));
#else
    matrix_rotate(m, angle, x, y, z);
#endif
}

void utils_matrix_scale(float* m, float x, float y, float z) {
#ifdef USE_CGLM
    glm_scale((vec4*)m, (vec3){x, y, z});
#elif defined(USE_RAYMATH)
    *((Matrix*)m) = MatrixMultiply(*((Matrix*)m), MatrixScale(x, y, z));
#else
    matrix_scale(m, x, y, z);
#endif
}

void utils_matrix_perspective(float* m, float fovy, float aspect, float near, float far) {
#ifdef USE_CGLM
    glm_perspective(fovy, aspect, near, far, (vec4*)m);
#elif defined(USE_RAYMATH)
    *((Matrix*)m) = MatrixPerspective(fovy * RAD2DEG, aspect, near, far);
#else
    matrix_perspective(m, fovy, aspect, near, far);
#endif
}

void utils_matrix_identity(float* m) {
#ifdef USE_CGLM
    glm_mat4_identity((vec4*)m);
#elif defined(USE_RAYMATH)
    *((Matrix*)m) = MatrixIdentity();
#else
    matrix_identity(m);
#endif
}

// Material properties
Material gold_material(void) {
    Material gold = {
        .ambient = {0.2473f, 0.1995f, 0.0745f, 1.0f},
        .diffuse = {0.7516f, 0.6065f, 0.2265f, 1.0f},
        .specular = {0.6283f, 0.5559f, 0.3661f, 1.0f},
        .shininess = 51.2f
    };
    return gold;
}

Material silver_material(void) {
    Material silver = {
        .ambient = {0.1923f, 0.1923f, 0.1923f, 1.0f},
        .diffuse = {0.5075f, 0.5075f, 0.5075f, 1.0f},
        .specular = {0.5083f, 0.5083f, 0.5083f, 1.0f},
        .shininess = 51.2f
    };
    return silver;
}

Material bronze_material(void) {
    Material bronze = {
        .ambient = {0.2125f, 0.1275f, 0.0540f, 1.0f},
        .diffuse = {0.7140f, 0.4284f, 0.1814f, 1.0f},
        .specular = {0.3936f, 0.2719f, 0.1667f, 1.0f},
        .shininess = 25.6f
    };
    return bronze;
}

// Additional utility functions
float utils_random_float(float min, float max) {
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

int utils_random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

float utils_lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float utils_clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
