#pragma once
#ifndef WORLD_RENDERER_HPP
#define WORLD_RENDERER_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/shader.hpp"
#include "utils/texture.hpp"

#include "world_buffer.hpp"

struct block_type_renderer_strct_t {
	GLuint texture_id; // Returned by glGenTextures
	GLuint texture_uniform;

	GLuint positions_buffer_id;
	GLuint uvs_buffer_id;
	GLuint normals_buffer_id;

	std::vector<float> positions_buffer;
	std::vector<float> uvs_buffer;
	std::vector<float> normals_buffer;
};

struct world_renderer_t {
	world_renderer_t(world_buffer_t &buffer);

	void init();

	void clear_preprocessing_data();
	// chunk_pos means position in plane XZ
	void preprocess_chunk(const glm::ivec2 &chunk_pos);
	void finish_preprocessing();

	void draw(
		const glm::vec3 &camera_pos,
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::mat4 &model_matrix
	);

	void deinit();

private:
	world_buffer_t &buffer;

	// Program
	GLuint program_id;

	// Uniforms
	GLuint MVP_matrix_uniform;
	GLuint view_matrix_uniform;
	GLuint model_matrix_uniform;
	GLuint projection_matrix_uniform;
	GLuint light_uniform;
	GLuint light_color_uniform;

	block_type_renderer_strct_t strcts[static_cast<uint8_t>(block_type::cnt)];

private:
	static constexpr size_t single_block_points_cnt = 6*2 * 3;
	static constexpr GLfloat single_block_positions[] = {
		1.000000,  1.000000,  1.000000,
		-1.000000,  1.000000, -1.000000,
		-1.000000,  1.000000,  1.000000,
		-1.000000,  1.000000, -1.000000,
		1.000000, -1.000000, -1.000000,
		-1.000000, -1.000000, -1.000000,
		1.000000,  1.000000, -1.000000,
		1.000000, -1.000000,  1.000000,
		1.000000, -1.000000, -1.000000,
		-1.000000, -1.000000,  1.000000,
		1.000000, -1.000000, -1.000000,
		1.000000, -1.000000,  1.000000,
		-1.000000,  1.000000,  1.000000,
		-1.000000, -1.000000, -1.000000,
		-1.000000, -1.000000,  1.000000,
		1.000000,  1.000000,  1.000000,
		-1.000000, -1.000000,  1.000000,
		1.000000, -1.000000,  1.000000,
		1.000000,  1.000000,  1.000000,
		1.000000,  1.000000, -1.000000,
		-1.000000,  1.000000, -1.000000,
		-1.000000,  1.000000, -1.000000,
		1.000000,  1.000000, -1.000000,
		1.000000, -1.000000, -1.000000,
		1.000000,  1.000000, -1.000000,
		1.000000,  1.000000,  1.000000,
		1.000000, -1.000000,  1.000000,
		-1.000000, -1.000000,  1.000000,
		-1.000000, -1.000000, -1.000000,
		1.000000, -1.000000, -1.000000,
		-1.000000,  1.000000,  1.000000,
		-1.000000,  1.000000, -1.000000,
		-1.000000, -1.000000, -1.000000,
		1.000000,  1.000000,  1.000000,
		-1.000000,  1.000000,  1.000000,
		-1.000000, -1.000000,  1.000000,
	};

	static constexpr GLfloat single_block_uv[] = {
		0.492188, -0.507812,
		0.257812, -0.742188,
		0.257812, -0.507812,
		0.257812, -0.757812,
		0.492188, -0.992188,
		0.257812, -0.992188,
		0.507812, -0.742188,
		0.742188, -0.507812,
		0.742188, -0.742188,
		0.257812, -0.242188,
		0.492188, -0.007812,
		0.492188, -0.242188,
		0.242188, -0.507812,
		0.007812, -0.742188,
		0.007812, -0.507812,
		0.492188, -0.492188,
		0.257812, -0.257812,
		0.492188, -0.257812,
		0.492188, -0.507812,
		0.492188, -0.742188,
		0.257812, -0.742188,
		0.257812, -0.757812,
		0.492188, -0.757812,
		0.492188, -0.992188,
		0.507812, -0.742188,
		0.507812, -0.507812,
		0.742188, -0.507812,
		0.257812, -0.242188,
		0.257812, -0.007812,
		0.492188, -0.007812,
		0.242188, -0.507812,
		0.242188, -0.742188,
		0.007812, -0.742188,
		0.492188, -0.492188,
		0.257812, -0.492188,
		0.257812, -0.257812,
	};

	static constexpr GLfloat single_block_normals[] = {
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		-0.000000,  0.000000, -1.000000,
		-0.000000,  0.000000, -1.000000,
		-0.000000,  0.000000, -1.000000,
		1.000000,  0.000000, -0.000000,
		1.000000,  0.000000, -0.000000,
		1.000000,  0.000000, -0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		-0.000000,  0.000000, -1.000000,
		-0.000000,  0.000000, -1.000000,
		-0.000000,  0.000000, -1.000000,
		1.000000,  0.000000, -0.000000,
		1.000000,  0.000000, -0.000000,
		1.000000,  0.000000, -0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
	};
};

#endif
