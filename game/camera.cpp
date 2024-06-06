// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "chunk.hpp"
#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include <useful.hpp>
#include "settings.hpp"

camera_t::camera_t(
		glm::vec3 pos,
		float horizontal_rotation_angle,
		float vertical_rotation_angle,
		float fov,
		float near_clip_plane_dist)
	:position{pos}
	,horizontal_rotation_angle{horizontal_rotation_angle}
	,vertical_rotation_angle{vertical_rotation_angle}
	,fov{fov}
	,near_clip_plane_dist{near_clip_plane_dist}
{
}

void camera_t::load_settings() {
    far_clip_plane_dist = global_settings.render_distance * chunk_t::WIDTH;
}

void camera_t::init_cyclicness(float cyclic_world_width_) {
    cyclic_world_width = cyclic_world_width_;
    normalize_cyclic_position();
}

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
    normalize_cyclic_position();
	if (get_following_mode())
		target_dist -= delta_time * moving_speed;
}
void camera_t::move_backward(float delta_time) {
	update_rotation_vectors();
	position -= direction_vec * delta_time * moving_speed;
    normalize_cyclic_position();
	if (get_following_mode())
		target_dist += delta_time * moving_speed;
}
void camera_t::move_right(float delta_time) {
	update_rotation_vectors();
	position += right_vec * delta_time * moving_speed;
    normalize_cyclic_position();
}
void camera_t::move_left(float delta_time) {
	update_rotation_vectors();
	position -= right_vec * delta_time * moving_speed;
    normalize_cyclic_position();
}

void camera_t::follow(float delta_time, glm::vec3 target) {
	update_rotation_vectors();

    // Find the closest cyclic target copy
    const glm::vec3 original_target = target;
    glm::vec3 candidate_target;
    float l_sq = len_sq(position - target);
    float candidate_l_sq;

    candidate_target = original_target - glm::vec3(cyclic_world_width, 0, 0);
    candidate_l_sq = len_sq(position - candidate_target);
    if (candidate_l_sq < l_sq) {
        target = candidate_target;
        l_sq = candidate_l_sq;
    }

    candidate_target = original_target + glm::vec3(cyclic_world_width, 0, 0);
    candidate_l_sq = len_sq(position - candidate_target);
    if (candidate_l_sq < l_sq) {
        target = candidate_target;
        l_sq = candidate_l_sq;
    }

    // Smoothly get closer to the target
	const glm::vec3 new_pos = target - direction_vec * target_dist;
	// position = new_pos; // Instant motion
	position = lerp(position, new_pos, delta_time*2.0f); // Smooth motion
    normalize_cyclic_position();
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
	const float halv_v_side = far_clip_plane_dist * std::tan(glm::radians(fov) * 0.5f);
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

void camera_t::normalize_cyclic_position() {
    position.x = mod_f(position.x, cyclic_world_width);
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
