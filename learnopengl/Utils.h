#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Conditional inclusions based on the graphics and math libraries being used
#ifdef USE_OPENGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#ifdef USE_RAYLIB
#include <raylib.h>
#endif

#ifdef USE_CGLM
#include <cglm/cglm.h>
#endif

#ifdef USE_RAYMATH
#include <raymath.h>
#endif

/**
 * @brief Structure representing material properties for lighting calculations
 */
typedef struct {
    float ambient[4];   /**< Ambient color of the material */
    float diffuse[4];   /**< Diffuse color of the material */
    float specular[4];  /**< Specular color of the material */
    float shininess;    /**< Shininess of the material */
} Material;

// File operations

/**
 * @brief Reads the contents of a file into a string
 * @param file_path Path to the file to be read
 * @return Dynamically allocated string containing the file contents, or NULL if an error occurred
 * @note The caller is responsible for freeing the returned string
 */
char* read_file(const char* file_path);

// OpenGL-specific functions
#ifdef USE_OPENGL

/**
 * @brief Checks for OpenGL errors
 * @return 1 if an error was found, 0 otherwise
 */
int check_opengl_error(void);

/**
 * @brief Prints the info log for a shader
 * @param shader The shader object to query
 */
void print_shader_log(unsigned int shader);

/**
 * @brief Prints the info log for a program
 * @param prog The program object to query
 */
void print_program_log(int prog);

/**
 * @brief Prepares a shader object
 * @param shader_type The type of shader (e.g., GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
 * @param shader_path Path to the shader source file
 * @return The created shader object, or 0 if an error occurred
 */
unsigned int prepare_shader(int shader_type, const char* shader_path);

/**
 * @brief Finalizes a shader program
 * @param program The program object to finalize
 * @return The finalized program object
 */
int finalize_shader_program(unsigned int program);

/**
 * @brief Creates a shader program from vertex and fragment shaders
 * @param vp Path to the vertex shader source file
 * @param fp Path to the fragment shader source file
 * @return The created shader program
 */
unsigned int create_shader_program(const char* vp, const char* fp);

/**
 * @brief Creates a shader program from vertex, geometry, and fragment shaders
 * @param vp Path to the vertex shader source file
 * @param gp Path to the geometry shader source file
 * @param fp Path to the fragment shader source file
 * @return The created shader program
 */
unsigned int create_shader_program_with_geometry(const char* vp, const char* gp, const char* fp);

#endif // USE_OPENGL

// Matrix operations (wrappers that work with different math libraries)

/**
 * @brief Multiplies two 4x4 matrices
 * @param m1 The first matrix
 * @param m2 The second matrix
 * @param result The resulting matrix
 */
void utils_matrix_multiply(float* m1, float* m2, float* result);

/**
 * @brief Applies a translation to a matrix
 * @param m The matrix to translate
 * @param x Translation along the x-axis
 * @param y Translation along the y-axis
 * @param z Translation along the z-axis
 */
void utils_matrix_translate(float* m, float x, float y, float z);

/**
 * @brief Applies a rotation to a matrix
 * @param m The matrix to rotate
 * @param angle The rotation angle in radians
 * @param x X component of the rotation axis
 * @param y Y component of the rotation axis
 * @param z Z component of the rotation axis
 */
void utils_matrix_rotate(float* m, float angle, float x, float y, float z);

/**
 * @brief Applies a scale transformation to a matrix
 * @param m The matrix to scale
 * @param x Scale factor for the x-axis
 * @param y Scale factor for the y-axis
 * @param z Scale factor for the z-axis
 */
void utils_matrix_scale(float* m, float x, float y, float z);

/**
 * @brief Creates a perspective projection matrix
 * @param m Pointer to a 16-element float array to store the resulting matrix
 * @param fovy Field of view angle in radians
 * @param aspect Aspect ratio of the viewport (width / height)
 * @param near Distance to the near clipping plane
 * @param far Distance to the far clipping plane
 */
void utils_matrix_perspective(float* m, float fovy, float aspect, float near, float far);

/**
 * @brief Initializes a matrix to the identity matrix
 * @param m Pointer to a 16-element float array to store the resulting matrix
 */
void utils_matrix_identity(float* m);

// Material properties

/**
 * @brief Creates a material with gold properties
 * @return The gold material
 */
Material gold_material(void);

/**
 * @brief Creates a material with silver properties
 * @return The silver material
 */
Material silver_material(void);

/**
 * @brief Creates a material with bronze properties
 * @return The bronze material
 */
Material bronze_material(void);

// Additional utility functions

/**
 * @brief Generates a random float within a specified range
 * @param min The minimum value of the range
 * @param max The maximum value of the range
 * @return A random float between min and max
 */
float utils_random_float(float min, float max);

/**
 * @brief Generates a random integer within a specified range
 * @param min The minimum value of the range
 * @param max The maximum value of the range
 * @return A random integer between min and max (inclusive)
 */
int utils_random_int(int min, int max);

/**
 * @brief Performs linear interpolation between two values
 * @param a The start value
 * @param b The end value
 * @param t The interpolation factor (0.0 to 1.0)
 * @return The interpolated value
 */
float utils_lerp(float a, float b, float t);

/**
 * @brief Clamps a value between a minimum and maximum
 * @param value The value to clamp
 * @param min The minimum allowed value
 * @param max The maximum allowed value
 * @return The clamped value
 */
float utils_clamp(float value, float min, float max);

#endif // UTILS_H
