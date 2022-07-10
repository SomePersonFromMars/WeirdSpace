#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct player_t {
	void init();

private:
	glm::vec3 position;

	GLuint texture_id;

	GLuint vao_id;

	GLuint positions_buffer_id;
	GLuint uvs_buffer_id;
	GLuint normals_buffer_id;

	std::vector<float> positions_buffer;
	std::vector<float> uvs_buffer;
	std::vector<float> normals_buffer;

	static constexpr float vertices_positions[] = {
		-0.000000, -2.000000,  1.000000,
		0.000000,  2.000000, -1.000000,
		0.000000,  2.000000,  1.000000,
		-0.000000, -2.000000,  1.000000,
		-0.000000, -2.000000, -1.000000,
		0.000000,  2.000000, -1.000000,
	};
	static constexpr float vertices_uvs[] = {
		0.000000,  0.000000,
		0.500000, -1.000000,
		-0.000000, -1.000000,
		0.000000,  0.000000,
		0.500000, -0.000000,
		0.500000, -1.000000,
	};
	static constexpr float vertices_normals[] = {
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
	};
};

#endif
