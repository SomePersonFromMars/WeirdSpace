#include "generator.hpp"

#include "settings.hpp"
#include "useful.hpp"
#include "shader_loader.hpp"

generator_D_t::generator_D_t() {
}

void generator_D_t::init() {
	// Load shaders and generate shader program
	GLuint compute_shader_id = compile_shader(
			SHADER_GENERATOR_D_COMPUTE_PATH, GL_COMPUTE_SHADER);

	program_id = link_program(1, compute_shader_id);

	delete_shader(compute_shader_id);
}

void generator_D_t::new_seed() {
}

void generator_D_t::load_settings() {
}

void generator_D_t::generate_bitmap(bitmap_t &bitmap) {
	// Shader
	glUseProgram(program_id);

	// Uniforms and texture
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, bitmap.get_texture_id());
	// glUniform1i(texture_sampler_uniform,
	// 		0);
	// glBindImageTexture(
	// 		0,
	// 		bitmap.get_texture_id(),
	// 		0,
	// 		GL_FALSE,
	// 		0,
	// 		GL_WRITE_ONLY,
	// 		)

	// Execute shader
}
