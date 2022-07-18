#pragma once
#ifndef WORLD_RENDERER_HPP
#define WORLD_RENDERER_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_A.hpp"
#include "world_buffer.hpp"

#include "settings.hpp"

struct world_renderer_t {
	world_renderer_t(
		shader_A_t &shader,
		world_buffer_t &buffer
	);

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
	shader_A_t &shader;
	world_buffer_t &buffer;

	// All blocks combined texture
	GLuint texture_id;

	// Vertex array object
	GLuint vao_id;

	GLuint positions_buffer_id;
	GLuint uvs_buffer_id;
	GLuint normals_buffer_id;
	GLuint positions_instanced_buffer_id;

	std::vector<float> positions_instanced_buffer;

private:
	static constexpr size_t single_block_points_cnt = 6*2 * 3;
	static constexpr GLfloat single_block_positions[] = {
		1.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  1.000000,
		1.000000,  1.000000,  1.000000,
		0.000000,  1.000000,  1.000000,
		0.000000,  0.000000,  0.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  1.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		0.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  1.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		1.000000,  0.000000,  1.000000,
		1.000000,  1.000000,  0.000000,
		1.000000,  0.000000,  1.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  1.000000,
		0.000000,  1.000000,  1.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  0.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		1.000000,  1.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  0.000000,
		1.000000,  1.000000,  1.000000,
		0.000000,  1.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		1.000000,  1.000000,  0.000000,
		1.000000,  1.000000,  1.000000,
		1.000000,  0.000000,  1.000000,
	};

	// For now only the sand texture is supported
	static constexpr GLfloat single_block_uvs[] = {
		0.999900, -0.000100,
		0.000100, -0.999900,
		0.000100, -0.000100,
		0.000100, -0.000100,
		0.999900, -0.999900,
		0.000100, -0.999900,
		0.999900, -0.999900,
		0.000100, -0.000100,
		0.999900, -0.000100,
		0.999900, -0.000100,
		0.000100, -0.999900,
		0.000100, -0.000100,
		0.000100, -0.000100,
		0.999900, -0.999900,
		0.000100, -0.999900,
		0.999900, -0.999900,
		0.000100, -0.000100,
		0.999900, -0.000100,
		0.999900, -0.000100,
		0.999900, -0.999900,
		0.000100, -0.999900,
		0.000100, -0.000100,
		0.999900, -0.000100,
		0.999900, -0.999900,
		0.999900, -0.999900,
		0.000100, -0.999900,
		0.000100, -0.000100,
		0.999900, -0.000100,
		0.999900, -0.999900,
		0.000100, -0.999900,
		0.000100, -0.000100,
		0.999900, -0.000100,
		0.999900, -0.999900,
		0.999900, -0.999900,
		0.000100, -0.999900,
		0.000100, -0.000100,
	};

	static constexpr GLfloat single_block_normals[] = {
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		0.000000,  1.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		-1.000000,  0.000000,  0.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000, -1.000000,  0.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
	};
};

#endif
