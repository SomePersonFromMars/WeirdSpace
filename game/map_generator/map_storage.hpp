#pragma once
#ifndef MAP_STORAGE_HPP
#define MAP_STORAGE_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <settings.hpp>

struct map_storage_t {
	// Basic usage functions
	map_storage_t() = default;
	void load_settings();
	void init_gl();
	void reallocate_gpu_and_cpu_memory();
	void draw(const glm::mat4 &MVP_matrix);
	void deinit_gl();
	~map_storage_t();

	// Getters and setters
	inline const int& get_width() const;
	inline const int& get_height() const;
	inline uint32_t get(int y, int x);
	void set(int y, int x, uint32_t color);
	void set(int y, int x, glm::u8vec3 color);
	void clear();

	// Fill texture in functions
	void load_to_texture(); // Load `content` to texture
	inline GLuint get_texture_id() const;

private:
	int width;
	int height;
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

inline const int& map_storage_t::get_width() const {
	return width;
}

inline const int& map_storage_t::get_height() const {
	return height;
}

inline uint8_t& map_storage_t::get(int y, int x, int component) {
	return content[y*width*3 + x*3 + component];
}

inline uint32_t map_storage_t::get(int y, int x) {
	if (x < 0 || x >= width)
		return 0;
	if (y < 0 || y >= height)
		return 0;
	return
		(uint32_t(get(y, x, 2)) << 0) |
		(uint32_t(get(y, x, 1)) << 8) |
		(uint32_t(get(y, x, 0)) << 16);
}

inline GLuint map_storage_t::get_texture_id() const {
	return texture_id;
}

#endif
