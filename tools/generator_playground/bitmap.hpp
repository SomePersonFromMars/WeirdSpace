#pragma once
#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "settings.hpp"

struct bitmap_t {
	// static constexpr int WIDTH = 256;
	// static constexpr int HEIGHT = 32*5;
	static constexpr int WIDTH = CHUNK_DIM*6 * 3;
	static constexpr int HEIGHT = CHUNK_DIM*3;
	// uint8_t content[HEIGHT][WIDTH][3] { };
	inline uint32_t get(int y, int x);
	void set(int y, int x, uint32_t color);
	void set(int y, int x, glm::u8vec3 color);
	void clear();

	bitmap_t() = default;
	void init();
	~bitmap_t();

	void load_to_texture();
	inline GLuint get_texture_id() const;
	void draw(const glm::mat4 &MVP_matrix);

private:
	uint8_t *content = nullptr;
	inline uint8_t& get(int y, int x, int component);

	GLuint texture_id;
	GLuint program_id;

	GLuint vao_id;
	GLuint quad_positions_buffer_id;
	GLuint quad_uvs_buffer_id;

	GLuint texture_sampler_uniform;
	GLuint MVP_matrix_uniform;

	static constexpr GLfloat quad_positions[] {
		-1, -1, 0,
		-1, 1, 0,
		1, -1, 0,
		1, 1, 0,
	};

	static constexpr GLfloat quad_uvs[] {
		0, 1,
		0, 0,
		1, 1,
		1, 0,
	};
};

inline uint8_t& bitmap_t::get(int y, int x, int component) {
	return content[y*WIDTH*3 + x*3 + component];
}

inline uint32_t bitmap_t::get(int y, int x) {
	return
		(uint32_t(get(y, x, 2)) << 0) |
		(uint32_t(get(y, x, 1)) << 8) |
		(uint32_t(get(y, x, 0)) << 16);
}

inline GLuint bitmap_t::get_texture_id() const {
	return texture_id;
}

#endif
