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
	inline uint32_t get_rgb_value(int y, int x) const;
	inline uint32_t get_rgba_value(int y, int x) const;
	inline uint8_t& get_component_reference(int y, int x, int component);
	inline uint8_t get_component_value(int y, int x, int component) const;
	inline void set_rgb_value(int y, int x, uint32_t color);
	inline void set_rgb_value(int y, int x, glm::u8vec3 color);
	void clear();
	void load_from_cpu_to_gpu_memory();
	void load_from_gpu_to_cpu_memory();
	inline GLuint get_texture_id() const;

private:
	int desired_width;
	int desired_height;
	int width = 0;
	int height = 0;
	uint8_t *content = nullptr;

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

inline uint8_t& map_storage_t::get_component_reference(
		int y, int x, int component) {
	assert(0 <= y && y < height);
	assert(0 <= x && x < width);
	return content[y*width*4 + x*4 + component];
}

inline uint8_t map_storage_t::get_component_value(
		int y, int x, int component) const {
	assert(0 <= y && y < height);
	assert(0 <= x && x < width);
	// if (x < 0 || x >= width) return 0;
	// if (y < 0 || y >= height) return 0;
	return content[y*width*4 + x*4 + component];
}

inline uint32_t map_storage_t::get_rgb_value(int y, int x) const {
	return
		(uint32_t(get_component_value(y, x, 2)) << 0) |
		(uint32_t(get_component_value(y, x, 1)) << 8) |
		(uint32_t(get_component_value(y, x, 0)) << 16);
}

inline uint32_t map_storage_t::get_rgba_value(int y, int x) const {
	return
		(uint32_t(get_component_value(y, x, 3)) << 0) |
		(uint32_t(get_component_value(y, x, 2)) << 8) |
		(uint32_t(get_component_value(y, x, 1)) << 16) |
		(uint32_t(get_component_value(y, x, 0)) << 24);
}

inline void map_storage_t::set_rgb_value(int y, int x, uint32_t color) {
	if (x < 0 || x >= width) return;
	if (y < 0 || y >= height) return;
	get_component_reference(y, x, 2) = (color & 0x0000ff) >> 0;
	get_component_reference(y, x, 1) = (color & 0x00ff00) >> 8;
	get_component_reference(y, x, 0) = (color & 0xff0000) >> 16;
}

inline void map_storage_t::set_rgb_value(int y, int x, glm::u8vec3 color) {
	if (x < 0 || x >= width) return;
	if (y < 0 || y >= height) return;
	get_component_reference(y, x, 2) = color.r;
	get_component_reference(y, x, 1) = color.g;
	get_component_reference(y, x, 0) = color.b;
}

inline GLuint map_storage_t::get_texture_id() const {
	return texture_id;
}

#endif
