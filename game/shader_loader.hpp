#pragma once
#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Loads, creates and compiles a shader
GLuint compile_shader(const char *file_path, GLenum shader_type);
void delete_shader(GLuint shader_id);

// Creates and links a program
GLuint link_program(size_t shaders_cnt, ...);
void delete_program(GLuint program_id);

#endif
