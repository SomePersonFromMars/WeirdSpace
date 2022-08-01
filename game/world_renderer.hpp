#pragma once
#ifndef WORLD_RENDERER_HPP
#define WORLD_RENDERER_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_world_B.hpp"
#include "world_buffer.hpp"

#include "settings.hpp"

struct world_renderer_t {
	world_renderer_t(
		shader_world_B_t &shader,
		world_buffer_t &buffer
	);

	void init();

	void clear_preprocessing_data();
	// chunk_pos means position in plane XZ
	void preprocess_chunk(const glm::ivec2 &chunk_pos);
	void finish_preprocessing();

	void draw(
		const glm::vec3 &light_pos,
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::mat4 &model_matrix
	);

	void deinit();

private:
	shader_world_B_t &shader;
	world_buffer_t &buffer;

	// All blocks combined texture
	GLuint texture_id;
	GLuint block_model_uniform_buffer_id;

	// Vertex array object
	GLuint vao_id;

	GLuint positions_instanced_buffer_id;
	GLuint blocks_types_instanced_buffer_id;
	GLuint faces_types_instanced_buffer_id;

	std::vector<float> positions_instanced_buffer;
	std::vector<uint8_t> blocks_types_instanced_buffer;
	std::vector<uint8_t> faces_types_instanced_buffer;

private:
	static constexpr size_t single_block_points_cnt = 6*2 * 3;

	// Faces order: Front, Top, Left, Right, Bottom, Back
	static constexpr GLfloat BLOCK_POSITIONS[] = {
		0, 0, 0,  0,
		0, 1, 0,  0,
		1, 0, 0,  0,
		1, 0, 0,  0,
		0, 1, 0,  0,
		1, 1, 0,  0,
		0, 1, 0,  0,
		0, 1, 1,  0,
		1, 1, 0,  0,
		1, 1, 0,  0,
		0, 1, 1,  0,
		1, 1, 1,  0,
		1, 0, 0,  0,
		1, 1, 0,  0,
		1, 0, 1,  0,
		1, 0, 1,  0,
		1, 1, 0,  0,
		1, 1, 1,  0,
		0, 0, 1,  0,
		0, 1, 1,  0,
		0, 0, 0,  0,
		0, 0, 0,  0,
		0, 1, 1,  0,
		0, 1, 0,  0,
		0, 0, 1,  0,
		0, 0, 0,  0,
		1, 0, 1,  0,
		1, 0, 1,  0,
		0, 0, 0,  0,
		1, 0, 0,  0,
		1, 0, 1,  0,
		1, 1, 1,  0,
		0, 0, 1,  0,
		0, 0, 1,  0,
		1, 1, 1,  0,
		0, 1, 1,  0,
	};

	// For now only the sand texture is supported
	static constexpr GLfloat BLOCK_UVS[] = {
		1, 0,  0, 0,
		1, 1,  0, 0,
		0, 0,  0, 0,
		0, 0,  0, 0,
		1, 1,  0, 0,
		0, 1,  0, 0,
		1, 0,  0, 0,
		1, 1,  0, 0,
		0, 0,  0, 0,
		0, 0,  0, 0,
		1, 1,  0, 0,
		0, 1,  0, 0,
		1, 0,  0, 0,
		1, 1,  0, 0,
		0, 0,  0, 0,
		0, 0,  0, 0,
		1, 1,  0, 0,
		0, 1,  0, 0,
		1, 0,  0, 0,
		1, 1,  0, 0,
		0, 0,  0, 0,
		0, 0,  0, 0,
		1, 1,  0, 0,
		0, 1,  0, 0,
		1, 0,  0, 0,
		1, 1,  0, 0,
		0, 0,  0, 0,
		0, 0,  0, 0,
		1, 1,  0, 0,
		0, 1,  0, 0,
		1, 0,  0, 0,
		1, 1,  0, 0,
		0, 0,  0, 0,
		0, 0,  0, 0,
		1, 1,  0, 0,
		0, 1,  0, 0,
	};

	static constexpr GLfloat BLOCK_NORMALS[] = {
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
	};
};

#endif
