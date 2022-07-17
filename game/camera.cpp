#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "utils/useful.hpp"

camera_t::camera_t(glm::vec3 pos, float h_angle, float v_angle, float fov)
	:position{pos}
	,horizontal_angle{h_angle}
	,vertical_angle{v_angle}
	,fov{fov}
{ }

void camera_t::rotate_up(float delta_time) {
	rotation_vectors_outdated = true;
	vertical_angle += rotation_speed * delta_time;
}
void camera_t::rotate_down(float delta_time) {
	rotation_vectors_outdated = true;
	vertical_angle -= rotation_speed * delta_time;
}
void camera_t::rotate_right(float delta_time) {
	rotation_vectors_outdated = true;
	horizontal_angle -= rotation_speed * delta_time;
}
void camera_t::rotate_left(float delta_time) {
	rotation_vectors_outdated = true;
	horizontal_angle += rotation_speed * delta_time;
}

void camera_t::move_forward(float delta_time) {
	update_rotation_vectors();
	position += direction_vec * delta_time * moving_speed;
	if (get_following_mode())
		target_dist -= delta_time * moving_speed;
}
void camera_t::move_backward(float delta_time) {
	update_rotation_vectors();
	position -= direction_vec * delta_time * moving_speed;
	if (get_following_mode())
		target_dist += delta_time * moving_speed;
}
void camera_t::move_right(float delta_time) {
	update_rotation_vectors();
	position += right_vec * delta_time * moving_speed;
}
void camera_t::move_left(float delta_time) {
	update_rotation_vectors();
	position -= right_vec * delta_time * moving_speed;
}

void camera_t::follow(float delta_time, glm::vec3 target) {
	update_rotation_vectors();
	// const glm::vec3 new_pos = target + glm::vec3(0, 3, -3);
	const glm::vec3 new_pos = target - direction_vec * target_dist;
	position = lerp(position, new_pos, delta_time*2.0f);
}

void camera_t::switch_following_mode() {
	following_mode = !following_mode;
}
void camera_t::enable_moving_acceleration(bool enable) {
	if (!enable)
		moving_speed = moving_speed_normal;
	else
		moving_speed = moving_speed_accelerated;
}
void camera_t::enable_rotation_acceleration(bool enable) {
	if (!enable)
		rotation_speed = rotation_speed_normal;
	else
		rotation_speed = rotation_speed_accelerated;
}

glm::mat4 camera_t::get_view_matrix() {
	update_rotation_vectors();
	return glm::lookAt( position, position+direction_vec, up_vec);
}
glm::mat4 camera_t::get_projection_matrix(
		GLint window_width, GLint window_height) const {
	return glm::perspective(
			glm::radians(fov),
			(float)window_width / (float)window_height, 0.1f, 400.0f
		);
}

void camera_t::update_rotation_vectors() {
	if (!rotation_vectors_outdated)
		return;

	// Direction
	direction_vec = glm::vec3(
			cos(vertical_angle) * sin(horizontal_angle),
			sin(vertical_angle),
			cos(vertical_angle) * cos(horizontal_angle)
			);

	// Right vector
	right_vec = glm::vec3(
			sin(horizontal_angle - 3.14f/2.0f),
			0,
			cos(horizontal_angle - 3.14f/2.0f)
			);

	// Up vector
	up_vec = glm::cross( right_vec, direction_vec );

	rotation_vectors_outdated = false;
}
