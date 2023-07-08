#pragma once
#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_world.hpp"

enum class block_type : uint8_t {
	none = 0,
	sand = 1,
	brick,

	cnt
};

struct chunk_t {
	static constexpr int WIDTH = 256;
	static constexpr int HEIGHT = 64;
	static constexpr int DEPTH = 256;

	block_type content[WIDTH][HEIGHT][DEPTH];
	// Neighbors order:
	// -x
	// +x
	// -y
	// +y
	// -z
	// +z
	const chunk_t *neighbors[6] { };

	static void init_gl_static(shader_world_t *pshader);
	static void deinit_gl_static();
	chunk_t();
	~chunk_t();

	void clear_preprocessing_data();
	void preprocess();
	void send_preprocessed_to_gpu();

	void draw(
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::mat4 &model_matrix,
		const glm::vec3 &light_pos
	);

private:
	static shader_world_t *pshader;

	// Static shader data
	static GLuint texture_id; // All blocks combined texture
	static GLuint block_model_uniform_buffer_id;

	// Per chunk shader data
	GLuint vao_id;

	GLuint positions_instanced_buffer_id;
	GLuint blocks_types_instanced_buffer_id;
	GLuint faces_types_instanced_buffer_id;

	std::vector<float> positions_instanced_buffer;
	std::vector<uint8_t> blocks_types_instanced_buffer;
	std::vector<uint8_t> faces_types_instanced_buffer;

private:

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

#define VERTEX_UV(off_x, off_y, mult_x, mult_y, x, y) \
	(off_x) + (mult_x)*static_cast<double>(x), \
	(off_y) + (mult_y)*static_cast<double>(y), \
	0, 0,
#define FACE_UVS(off_x, off_y, mult_x, mult_y) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 1, 1) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 1, 0) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 0, 1) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 0, 1) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 1, 0) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 0, 0)

#define SAND_FACE_UVS \
	FACE_UVS(0.0, 0, 0.125, 1.0)

#define BRICK_FACE_UVS \
	FACE_UVS(0.375, 0, 0.125, 1.0)

	static constexpr GLfloat BLOCK_UVS[] = {
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
	};

#undef SAND_FACE_UVS
#undef BRICK_FACE_UVS
#undef FACES_UVS
#undef VERTEX_UV

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
