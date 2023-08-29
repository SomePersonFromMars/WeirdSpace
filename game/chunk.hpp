#pragma once
#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>

#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_world.hpp"
#include <geometry.hpp>

enum class block_type : uint8_t {
	none = 0,
	sand = 1,
	brick,

	cnt
};

struct chunk_t {
	static constexpr int WIDTH = 256;
	static constexpr int HEIGHT = 128;
	static constexpr int DEPTH = 256;
	static const glm::ivec3 DIMENSIONS;

	block_type content[WIDTH][HEIGHT][DEPTH];
	// Neighbors order:
	// 0,  1,  2,  3,  4,  5
	// -x, +x, -y, +y, -z, +z
	const chunk_t *neighbors[6] { };

	static void init_gl_static(shader_world_t *pshader);
	static void deinit_gl_static();
	chunk_t();
	~chunk_t();

	void clear_cpu_preprocessing_data();
	void preprocess_on_cpu();
	void send_preprocessed_to_gpu();
	inline bool is_rendering_enabled() const;

	void draw(
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::mat4 &model_matrix,
		const glm::vec3 &light_pos
	) const;

	void draw_if_visible(
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::vec3 &light_pos,
		const glm::vec3 &buffer_chunk_position_XYZ,
		const frustum_t &camera_frustum
	);

private:
	bool preprocessing_data_available = false;
	bool rendering_enabled = true;
	inline void enable_rendering(bool enable);

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

inline void chunk_t::enable_rendering(bool enable) {
	rendering_enabled = enable;
}

inline bool chunk_t::is_rendering_enabled() const {
	return rendering_enabled;
}

#endif
