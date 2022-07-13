#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct camera_t {
	// Constructor
	camera_t(glm::vec3 pos, float h_angle, float v_angle, float fov);

	// Speed
	// `x` units per second
	static constexpr float moving_speed_normal = 8.0f;
	static constexpr float moving_speed_accelerated = 64.0f;
	static constexpr float rotation_speed_normal = 2.0f;
	static constexpr float rotation_speed_accelerated = 0.5f;

	// Interface
	void rotate_up     (float delta_time);
	void rotate_down   (float delta_time);
	void rotate_right  (float delta_time);
	void rotate_left   (float delta_time);
	void move_forward  (float delta_time);
	void move_backward (float delta_time);
	void move_right    (float delta_time);
	void move_left     (float delta_time);
	void enable_moving_acceleration   (bool enable);
	void enable_rotation_acceleration (bool enable);

	inline const glm::vec3& get_position() const;
	inline const float get_horizontal_angle() const;
	inline const float get_vertical_angle() const;
	glm::mat4 get_view_matrix();
	glm::mat4 get_projection_matrix(
			GLint window_width, GLint window_height) const;

private:
	// Static state
	glm::vec3 position;
	float horizontal_angle;
	float vertical_angle;
	float fov;

	// Kinematic state
	float moving_speed = moving_speed_normal;
	float rotation_speed = rotation_speed_normal;

	// Rotation vectors
	bool rotation_vectors_outdated = true;
	glm::vec3 direction_vec;
	glm::vec3 right_vec;
	glm::vec3 up_vec;

	void update_rotation_vectors();
};

inline const glm::vec3& camera_t::get_position() const {
	return position;
}
inline const float camera_t::get_horizontal_angle() const {
	return horizontal_angle;
}
inline const float camera_t::get_vertical_angle() const {
	return vertical_angle;
}

#endif
