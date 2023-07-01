#include "generator.hpp"

#include "settings.hpp"
#include "useful.hpp"
#include "shader_loader.hpp"

#include <chrono>

void generator_C_t::init() {
#ifdef GENERATE_WITH_GPU
	// Load shaders and generate shader programs
	GLuint compute_shader_id = compile_shader(
			SHADER_GENERATOR_D_COMPUTE_PATH, GL_COMPUTE_SHADER);
	program1 = link_program(1, compute_shader_id);
	delete_shader(compute_shader_id);

	GLuint vertex_shader_id = compile_shader(
			SHADER_GENERATOR_D_VERRTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_GENERATOR_D_FRAGMENT_PATH, GL_FRAGMENT_SHADER);
	program2 = link_program(2, vertex_shader_id, fragment_shader_id);
	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Uniforms
	t_uniform = glGetUniformLocation(program1, "t");

	// Create FBO etc.
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			bitmap->get_texture_id(), 0);
	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Continents vao etc.
	glGenVertexArrays(1, &continents_vao);
	glBindVertexArray(continents_vao);

	// Buffers
	static constexpr float vertices_pos[] {
		-0.5, -0.5,
		0.0,  0.5,
		0.5, -0.5,
	};
	static constexpr uint8_t types[] {
		2
	};
	glGenBuffers(1, &continents_triangle_pos_buf);
	glGenBuffers(1, &continents_type_buf);

	glBindBuffer(GL_ARRAY_BUFFER, continents_triangle_pos_buf);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_pos),
			vertices_pos,
			GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, continents_type_buf);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(types),
			types,
			GL_STATIC_DRAW);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, NULL);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
#endif
}

void generator_C_t::deinit() {
#ifdef GENERATE_WITH_GPU
	glDeleteProgram(program1);
	glDeleteProgram(program2);
	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &continents_vao);
#endif
}


void generator_C_t::draw_map_gpu() {
#ifdef GENERATE_WITH_GPU
#define PROG 1
#if PROG == 1
	// Shader program
	glUseProgram(program1);

	// Uniforms and texture
	glBindImageTexture(
			0,
			bitmap->get_texture_id(),
			0,
			GL_FALSE,
			0,
			GL_WRITE_ONLY,
			GL_RGBA8
			);

	const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();
	static auto app_start_ms = now_ms;
	const auto elapsed_ms = now_ms - app_start_ms;
	const float t = float(elapsed_ms);
	// PRINT_F(t);

	glUniform1f(t_uniform, t);

	// Execute shader
	glDispatchCompute(bitmap->WIDTH/3, bitmap->HEIGHT, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	// glFinish();

#elif PROG == 2
	glUseProgram(program2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindVertexArray(continents_vao);

	glViewport(0, 0, width, height);
	static const GLfloat blue[] = { 0.0f, 0.0f, 0.3f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, blue);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	// GL_GET_ERROR;
#endif
#endif
}
