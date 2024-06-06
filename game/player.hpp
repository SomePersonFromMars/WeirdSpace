// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_A.hpp"
#include "world_buffer.hpp"

#include <useful.hpp>
#include <settings.hpp>

struct player_t {
	player_t(shader_A_t &shader, world_buffer_t &world_buffer);

	glm::vec3 debug_position;

	// Rendering
	void init_gl();
	void deinit_gl();
	void draw_cyclic(
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
        const shader_A_fragment_common_uniforms_t &common_uniforms
	);
	void draw_single(
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix,
        const float      off_x,
        const shader_A_fragment_common_uniforms_t &common_uniforms
	);

	// Movement
	inline void set_position(glm::vec3 new_pos);
	inline const glm::vec3& get_position() const;
	inline void enable_moving_acceleration(bool enable);

	void move_up        (float delta_time);
	void move_down      (float delta_time);
	void move_right     (float delta_time);
	void move_left      (float delta_time);
	void jump           (float delta_time);
	void update_physics (float delta_time);
	inline void switch_fly_mode();

private:
	shader_A_t &shader;
	world_buffer_t &world_buffer;

	const float move_speed_normal = 3.0f;
	const float move_speed_accelerated = 30.0f;
	float move_speed = move_speed_normal;

	// Right-bottom-front corner of the player's hitbox
	glm::vec3 position;
	bool fly_mode = false;
	glm::vec2 speed;
	glm::vec2 frame_offset = glm::vec2(0, 0);

	// Move player down axis with collisions checking
	bool on_axis_move_by(float offset, float glm::vec3::* axis_ptr);
	// Move player down any XY vector with collisions checking
	// and probabilistic gap entering
	glm::bvec2 move_by(glm::vec2 offset);
	inline void move_by_queued(glm::vec2 offset);	// Cumulates offset
													// to flush it every frame

	GLuint texture_id;
	GLuint vao_id;
	GLuint positions_buffer_id;
	GLuint uvs_buffer_id;
	GLuint normals_buffer_id;
	GLuint positions_instanced_buffer_id;

	static constexpr float max_offset_len = 0.1f;	// Maximum atomic offset
													// vector length
	static const glm::vec2 hitbox_dimensions;

	static constexpr float vertices_positions[] = {
		0, 0, 0,
		0, 2, 0,
		1, 0, 0,
		1, 0, 0,
		0, 2, 0,
		1, 2, 0,
		0, 0, 0,
		1, 0, 0,
		0, 2, 0,
		1, 0, 0,
		1, 2, 0,
		0, 2, 0,
	};

	static constexpr float vertices_uvs[] = {
		0.5, 1.0,
		0.5, 0.0,
		0.0, 1.0,
		0.0, 1.0,
		0.5, 0.0,
		0.0, 0.0,
		0.5, 1.0,
		0.0, 1.0,
		0.5, 0.0,
		0.0, 1.0,
		0.0, 0.0,
		0.5, 0.0,
	};

	static constexpr float vertices_normals[] = {
		0,  0, -1,
		0,  0, -1,
		0,  0, -1,
		0,  0, -1,
		0,  0, -1,
		0,  0, -1,
		0,  0,  1,
		0,  0,  1,
		0,  0,  1,
		0,  0,  1,
		0,  0,  1,
		0,  0,  1,
	};
};

inline void player_t::set_position(glm::vec3 new_pos) {
	position = new_pos;
}

inline const glm::vec3& player_t::get_position() const {
	return position;
}

inline void player_t::enable_moving_acceleration(bool enable) {
	if (enable)
		move_speed = move_speed_accelerated;
	else
		move_speed = move_speed_normal;
}

inline void player_t::switch_fly_mode() {
	fly_mode = !fly_mode;
}

inline void player_t::move_by_queued(glm::vec2 offset) {
	frame_offset += offset;
}

#endif
