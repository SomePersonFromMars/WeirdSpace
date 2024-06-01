#include "player.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "texture_loader.hpp"

#ifdef DEBUG
	#include <cstdio>
#endif

const glm::vec2 player_t::hitbox_dimensions(0.8, 1.7);

player_t::player_t(shader_A_t &shader, world_buffer_t &world_buffer)
	:shader{shader}
	,world_buffer{world_buffer}
{  }

void player_t::init_gl() {
	// Generate OpenGL ids
	texture_id = load_texture(TEXTURE_PLAYER_PATH);

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &positions_buffer_id);
	glGenBuffers(1, &uvs_buffer_id);
	glGenBuffers(1, &normals_buffer_id);
	glGenBuffers(1, &positions_instanced_buffer_id);

	// Initialize VBOs with single instance data
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_positions),
			vertices_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_uvs),
			vertices_uvs, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(vertices_normals),
			vertices_normals, GL_STATIC_DRAW);

	const glm::vec3 dummy_pos(0, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(dummy_pos),
			&dummy_pos, GL_STATIC_DRAW);

	// Add shader vertex attributes to the VAO
	// 1rst attribute buffer: vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer: UV coordinates
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 3rd attribute buffer: normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 4th attribute buffer: vertices
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0); // Unbind vao, not necessary
}

void player_t::deinit_gl() {
	glDeleteBuffers(1,  &positions_buffer_id);
	glDeleteBuffers(1,  &uvs_buffer_id);
	glDeleteBuffers(1,  &normals_buffer_id);
	glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteTextures(1, &texture_id);

	glDeleteVertexArrays(1, &vao_id);
}

void player_t::draw_cyclic(
	const glm::vec3 &light_pos,
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix
	) {
    draw_single(light_pos, projection_matrix, view_matrix, world_buffer.get_buffer_width() * chunk_t::WIDTH);
    draw_single(light_pos, projection_matrix, view_matrix, 0);
    draw_single(light_pos, projection_matrix, view_matrix, -world_buffer.get_buffer_width() * chunk_t::WIDTH);
}

void player_t::draw_single(
	const glm::vec3 &light_pos,
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix,
    const float      off_x
	) {
	// Shader
	glUseProgram(shader.program_id);

	// Set uniforms
	glm::mat4 model_matrix(1);
	model_matrix = glm::translate(
			model_matrix,
			position + glm::vec3((1.0f-hitbox_dimensions.x)/-2.0f, 0, 0) + glm::vec3(off_x, 0, 0)
		);

	glUniformMatrix4fv(shader.model_matrix_uniform,
			1, GL_FALSE, &model_matrix[0][0]);
	glUniformMatrix4fv(shader.view_matrix_uniform,
			1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(shader.projection_matrix_uniform,
			1, GL_FALSE, &projection_matrix[0][0]);

	const glm::vec3 light_color = color_hex_to_vec3(LIGHT_COLOR);
	glUniform3f(shader.light_pos_worldspace_uniform,
			light_pos.x, light_pos.y, light_pos.z);
	glUniform3f(shader.light_color_uniform,
			light_color.x,
			light_color.y,
			light_color.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(shader.texture_sampler_uniform,
			0);

	// Bind VAO and draw
	glBindVertexArray(vao_id);
	glDrawArraysInstanced(GL_TRIANGLES,
		0, ARR_SIZE(vertices_positions)/3,
		1
	);
}

void player_t::move_up(float delta_time) {
	move_by_queued({0, move_speed * delta_time});
}

void player_t::move_down(float delta_time) {
	move_by_queued({0, -move_speed * delta_time});
}

void player_t::move_right(float delta_time) {
	move_by_queued({-move_speed * delta_time, 0});
}

void player_t::move_left(float delta_time) {
	move_by_queued({move_speed * delta_time, 0});
}

void player_t::jump([[maybe_unused]] float delta_time) {
	if (position.y == std::floor(position.y)
			&& world_buffer.get(glm::ivec3(
					std::floor(position.x),
					std::floor(position.y)-1,
					std::floor(position.z)
				)) != block_type::none)
	speed.y = 10.0f;
}
#undef move_speed

bool player_t::on_axis_move_by(float offset, float glm::vec3::* axis_ptr) {
	if (offset == 0)
		return false;

	// PRINT_F(offset);

	float constrained_offset;
	float free_offset;
	float constrained_pos_comp; // Constrained position's component

	float colliding_point_pos_comp = position.*axis_ptr;
	float hitbox_dim_comp =
		glm::vec3(hitbox_dimensions.x, hitbox_dimensions.y, 0).*axis_ptr;
	if (offset > 0) {
		colliding_point_pos_comp += hitbox_dim_comp;

		// Not sure: Distract an epsilon from this value?
		constrained_pos_comp
			= std::ceil(colliding_point_pos_comp) - hitbox_dim_comp;
		constrained_offset
			= std::ceil(colliding_point_pos_comp) - colliding_point_pos_comp;

		free_offset = 1;
	} else {
		constrained_pos_comp
			= std::floor(colliding_point_pos_comp);
		constrained_offset
			= constrained_pos_comp - colliding_point_pos_comp;

		free_offset = -1;
	}
	free_offset += constrained_offset;

	if (std::abs(offset) < std::abs(free_offset))
		free_offset = offset;
	if (std::abs(free_offset) <= std::abs(constrained_offset)) {
		position.*axis_ptr += free_offset;
		return false;
	}

	glm::vec3 free_pos = position;
	free_pos.*axis_ptr += free_offset;
	if (world_buffer.collision_check_XY_rect(free_pos, hitbox_dimensions)) {
		position.*axis_ptr = constrained_pos_comp;
		return true;
	} else {
		position.*axis_ptr += free_offset;
		return false;
	}
}

glm::bvec2 player_t::move_by(glm::vec2 offset) {
	if (offset == glm::vec2(0, 0))
		return {false, false};

	// PRINT_F(offset.x);

	const float off_len_sq = len_sq(offset);
	const float ratio = std::sqrt(max_offset_len*max_offset_len / off_len_sq);

	const glm::vec2 atomic_offset(offset.x * ratio, offset.y * ratio);
	const int loops_cnt = std::ceil(1.0f / ratio);

	glm::bvec2 has_collided(false, false);
	for (int i = 0; i < loops_cnt; ++i) {
		glm::vec2 cur_off = atomic_offset;
		if (len_sq(offset) < max_offset_len*max_offset_len)
			cur_off = offset;

		has_collided.x = has_collided.x
			|| on_axis_move_by(cur_off.x, &glm::vec3::x);
		has_collided.y = has_collided.y
			|| on_axis_move_by(cur_off.y, &glm::vec3::y);

		offset -= cur_off;
	}

	position.x = mod_f(position.x,
			static_cast<float>(world_buffer.get_buffer_width()*chunk_t::WIDTH));
	return has_collided;
}

void player_t::update_physics(float delta_time) {
	if (!fly_mode)
		// Gravity
		speed.y -= delta_time * 30.0f;
	move_by_queued(glm::vec2(0, speed.y * delta_time));

	glm::bvec2 has_collided = move_by(frame_offset);
	if (has_collided.y)
		speed.y = 0.0f;
	frame_offset = glm::vec2(0, 0);
}
