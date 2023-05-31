#include "generator.hpp"

#include "settings.hpp"
#include "useful.hpp"
#include "shader_loader.hpp"

#include <chrono>

generator_D_t::generator_D_t() {
}

void generator_D_t::init() {
	// Load shaders and generate shader program
	GLuint compute_shader_id = compile_shader(
			SHADER_GENERATOR_D_COMPUTE_PATH, GL_COMPUTE_SHADER);

	program_id = link_program(1, compute_shader_id);

	delete_shader(compute_shader_id);

	// Uniforms
	t_uniform = glGetUniformLocation(program_id, "t");
}

void generator_D_t::new_seed() {
}

void generator_D_t::load_settings() {
}

void generator_D_t::generate_bitmap(bitmap_t &bitmap) {
	// WHERE;
	// PRINT_U(glGetError());

	// Shader
	glUseProgram(program_id);

	// Uniforms and texture
	glBindImageTexture(
			0,
			bitmap.get_texture_id(),
			0,
			GL_FALSE,
			0,
			GL_WRITE_ONLY,
			GL_RGBA8
			);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, bitmap.get_texture_id());
	// glUniform1i(texture_sampler_uniform,
	//		0);
	// glBindImageTexture(
	//		0,
	//		bitmap.get_texture_id(),
	//		0,
	//		GL_FALSE,
	//		0,
	//		GL_WRITE_ONLY,
	//		)

	const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();
	static auto app_start_ms = now_ms;
	const auto elapsed_ms = now_ms - app_start_ms;
	const float t = float(elapsed_ms);
	// PRINT_F(t);

	glUniform1f(t_uniform, t);

	// Execute shader
	// WHERE; PRINT_U(glGetError());
	glDispatchCompute(bitmap.WIDTH/3, bitmap.HEIGHT, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	// glFinish();

	// WHERE; PRINT_U(glGetError());
}
