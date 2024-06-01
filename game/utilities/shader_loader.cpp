#include "shader_loader.hpp"

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <cstdlib>
#include <cstring>
#include <cstdarg>

GLuint compile_shader(const char *file_path, GLenum shader_type) {
	// Read the Vertex Shader source from the file
	std::string shader_source;
	std::ifstream shader_source_fstream(file_path, std::ios::in);
	if (shader_source_fstream.is_open()) {
		std::stringstream sstr;
		sstr << shader_source_fstream.rdbuf();
		shader_source = sstr.str();
		shader_source_fstream.close();
	} else {
		fprintf(stderr, "Impossible to open %s.\n", file_path);
		getchar();
		return 0;
	}

	// Create shader
	GLuint shader_id = glCreateShader(shader_type);

	// Compile Vertex Shader
	printf("Compiling shader: %s\n", file_path);
	char const *shader_source_ptr = shader_source.c_str();
	glShaderSource(shader_id, 1, &shader_source_ptr , NULL);
	glCompileShader(shader_id);

	// Check Vertex Shader
	GLint
		compile_status,
		info_log_length;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> shader_infor_log(info_log_length+1);
		glGetShaderInfoLog(shader_id,
				info_log_length, NULL, &shader_infor_log[0]);
		printf("%s\n", &shader_infor_log[0]);
	}

	return shader_id;
}
void delete_shader(GLuint shader_id) {
	glDeleteShader(shader_id);
}

GLuint link_program(size_t shaders_cnt, ...) {
	std::va_list args;
	va_start(args, shaders_cnt);
	vector<GLuint> shaders_ids(shaders_cnt);
	for (GLuint &shader_id : shaders_ids)
		shader_id = va_arg(args, GLuint);
	va_end(args);

	// Link the program
	printf("Linking program\n");
	GLuint program_id = glCreateProgram();
	for (const GLuint shader_id : shaders_ids)
		glAttachShader(program_id, shader_id);
	glLinkProgram(program_id);

	// Check for errors
	GLint
		link_status,
		info_log_length;
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		vector<char> program_info_log(info_log_length);
		glGetProgramInfoLog(program_id,
				info_log_length, nullptr, &program_info_log[0]);
		printf("%s\n", &program_info_log[0]);
	}

	// Detach shaders
	for (const GLuint shader_id : shaders_ids)
		glDetachShader(program_id, shader_id);

	return program_id;
}
void delete_program(GLuint program_id) {
	glDeleteProgram(program_id);
}
