#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include <useful.hpp>

camera_t::camera_t(
		glm::vec3 pos,
		float horizontal_rotation_angle,
		float vertical_rotation_angle,
		float fov,
		float near_clip_plane_dist,
		float far_clip_plane_dist)
	:position{pos}
	,horizontal_rotation_angle{horizontal_rotation_angle}
	,vertical_rotation_angle{vertical_rotation_angle}
	,fov{fov}
	,near_clip_plane_dist{near_clip_plane_dist}
	,far_clip_plane_dist{far_clip_plane_dist}
{ }

void camera_t::rotate_up(float delta_time) {
	rotation_vectors_outdated = true;
	vertical_rotation_angle += rotation_speed * delta_time;
}
void camera_t::rotate_down(float delta_time) {
	rotation_vectors_outdated = true;
	vertical_rotation_angle -= rotation_speed * delta_time;
}
void camera_t::rotate_right(float delta_time) {
	rotation_vectors_outdated = true;
	horizontal_rotation_angle -= rotation_speed * delta_time;
}
void camera_t::rotate_left(float delta_time) {
	rotation_vectors_outdated = true;
	horizontal_rotation_angle += rotation_speed * delta_time;
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

	const glm::vec3 new_pos = target - direction_vec * target_dist;
	// position = new_pos; // Instant motion
	position = lerp(position, new_pos, delta_time*2.0f); // Smooth motion
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

glm::mat4 camera_t::calculate_view_matrix() {
	update_rotation_vectors();
	return glm::lookAt( position, position+direction_vec, up_vec);
}

glm::mat4 camera_t::calculate_projection_matrix(float aspect) const {
	return glm::perspective(
			glm::radians(fov),
			aspect,
			near_clip_plane_dist,
			far_clip_plane_dist
		);
}

frustum_t camera_t::calculate_frustum_planes(float aspect) {
	update_rotation_vectors();

	frustum_t frustum;
	const float halv_v_side = far_clip_plane_dist * std::tan(fov * 0.5f);
	const float half_h_side = halv_v_side * aspect;
	const glm::vec3 front_mult_far = far_clip_plane_dist * direction_vec;

	frustum.near_face = {
		get_position() + near_clip_plane_dist * direction_vec,
		direction_vec };
	frustum.far_face = {
		get_position() + front_mult_far,
		-direction_vec };
	frustum.right_face = {
		get_position(),
		glm::cross(front_mult_far - right_vec * half_h_side, up_vec) };
	frustum.left_face = {
		get_position(),
		glm::cross(up_vec,front_mult_far + right_vec * half_h_side) };
	frustum.top_face = {
		get_position(),
		glm::cross(right_vec, front_mult_far - up_vec * halv_v_side) };
	frustum.bottom_face = {
		get_position(),
		glm::cross(front_mult_far + up_vec * halv_v_side, right_vec) };

	return frustum;
}

void camera_t::update_rotation_vectors() {
	if (!rotation_vectors_outdated)
		return;

	// Direction
	direction_vec = glm::vec3(
			cos(vertical_rotation_angle) * sin(horizontal_rotation_angle),
			sin(vertical_rotation_angle),
			cos(vertical_rotation_angle) * cos(horizontal_rotation_angle)
			);

	// Right vector
	right_vec = glm::vec3(
			sin(horizontal_rotation_angle - 3.14f/2.0f),
			0,
			cos(horizontal_rotation_angle - 3.14f/2.0f)
			);

	// Up vector
	up_vec = glm::cross( right_vec, direction_vec );

	rotation_vectors_outdated = false;
}
