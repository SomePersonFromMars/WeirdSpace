#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_A.hpp"

#include "utils/useful.hpp"
#include "settings.hpp"

struct player_t {
	player_t(shader_A_t &shader);

	void init();
	inline void set_position(glm::vec3 new_pos) {
		position = new_pos;
	}

	void draw(
		const glm::vec3 &camera_pos,
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::mat4 &model_matrix
	);

	void deinit();

private:
	shader_A_t &shader;

	glm::vec3 position;

	GLuint texture_id;

	GLuint vao_id;

	GLuint positions_buffer_id;
	GLuint uvs_buffer_id;
	GLuint normals_buffer_id;
	GLuint positions_instanced_buffer_id;

	static constexpr float vertices_positions[] = {
		-0.000000, -2.000000,  1.000000,
		0.000000,  2.000000, -1.000000,
		0.000000,  2.000000,  1.000000,
		-0.000000, -2.000000,  1.000000,
		-0.000000, -2.000000, -1.000000,
		0.000000,  2.000000, -1.000000,
		0.000000, -2.000000, -1.000000,
		0.000000,  2.000000,  1.000000,
		-0.000000,  2.000000, -1.000000,
		0.000000, -2.000000, -1.000000,
		0.000000, -2.000000,  1.000000,
		0.000000,  2.000000,  1.000000,
	};
	static constexpr float vertices_uvs[] = {
		0.000000,  0.000000,
		0.500000, -0.984375,
		-0.000000, -0.984375,
		0.000000,  0.000000,
		0.500000, -0.000000,
		0.500000, -0.984375,
		0.500000,  0.000000,
		-0.000000, -0.984375,
		0.500000, -0.984375,
		0.500000,  0.000000,
		0.000000, -0.000000,
		-0.000000, -0.984375,
	};
	static constexpr float vertices_normals[] = {
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		1.000000, -0.000000,  0.000000,
		-1.000000, -0.000000,  0.000000,
		-1.000000, -0.000000,  0.000000,
		-1.000000, -0.000000,  0.000000,
		-1.000000, -0.000000,  0.000000,
		-1.000000, -0.000000,  0.000000,
		-1.000000, -0.000000,  0.000000,
	};
};

#endif
