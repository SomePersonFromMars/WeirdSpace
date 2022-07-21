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

	glm::vec3 debug_position;

	// Rendering
	void init();
	void deinit();
	void draw(
		const glm::vec3 &light_pos,
		const glm::mat4 &projection_matrix,
		const glm::mat4 &view_matrix
	);

	// Movement
	inline void set_position(glm::vec3 new_pos);
	inline const glm::vec3& get_position() const;

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

	// Right-bottom-front corner of the player's hitbox
	glm::vec3 position;
	bool fly_mode = true;
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
		1.000000,  0.000000,  0.000000,
		-0.000000,  2.000000, -0.000000,
		1.000000,  2.000000,  0.000000,
		1.000000,  0.000000,  0.000000,
		0.000000,  0.000000, -0.000000,
		-0.000000,  2.000000, -0.000000,
		0.000000,  0.000000,  0.000000,
		1.000000,  2.000000, -0.000000,
		-0.000000,  2.000000,  0.000000,
		0.000000,  0.000000,  0.000000,
		1.000000,  0.000000, -0.000000,
		1.000000,  2.000000, -0.000000,
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
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000, -1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
		0.000000,  0.000000,  1.000000,
	};
};

inline void player_t::set_position(glm::vec3 new_pos) {
	position = new_pos;
}

inline const glm::vec3& player_t::get_position() const {
	return position;
}

inline void player_t::switch_fly_mode() {
	fly_mode = !fly_mode;
}

inline void player_t::move_by_queued(glm::vec2 offset) {
	frame_offset += offset;
}

#endif
