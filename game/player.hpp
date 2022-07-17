#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_A.hpp"
#include "world_buffer.hpp"

#include "utils/useful.hpp"
#include "settings.hpp"

struct player_t {
	player_t(shader_A_t &shader, world_buffer_t &world_buffer);

	// Rendering
	void init();
	void deinit();
	void draw(
		const glm::vec3 &camera_pos,
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
		const glm::mat4 &model_matrix
	);

	// Movement
	inline void set_position(glm::vec3 new_pos);
	inline const glm::vec3& get_position() const;
	glm::vec2 move_by(glm::vec2 offset);

	void move_up        (float delta_time);
	void move_down      (float delta_time);
	void move_right     (float delta_time);
	void move_left      (float delta_time);
	void jump           (float delta_time);
	void update_physics (float delta_time);

private:
	shader_A_t &shader;
	world_buffer_t &world_buffer;

	glm::vec3 position;
	glm::vec3 speed;

	GLuint texture_id;
	GLuint vao_id;
	GLuint positions_buffer_id;
	GLuint uvs_buffer_id;
	GLuint normals_buffer_id;
	GLuint positions_instanced_buffer_id;

	static constexpr float vertices_positions[] = {
		-0.500000,  0.000000,  0.000000,
		0.500000,  2.000000, -0.000000,
		-0.500000,  2.000000,  0.000000,
		-0.500000,  0.000000,  0.000000,
		0.500000,  0.000000, -0.000000,
		0.500000,  2.000000, -0.000000,
		0.500000,  0.000000,  0.000000,
		-0.500000,  2.000000, -0.000000,
		0.500000,  2.000000,  0.000000,
		0.500000,  0.000000,  0.000000,
		-0.500000,  0.000000, -0.000000,
		-0.500000,  2.000000, -0.000000,
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
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
	};
};

inline void player_t::set_position(glm::vec3 new_pos) {
	position = new_pos;
}

inline const glm::vec3& player_t::get_position() const {
	return position;
}

#endif
