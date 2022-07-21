#include "player.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "utils/shader.hpp"
#include "utils/texture.hpp"

const glm::vec2 player_t::hitbox_dimensions(0.8, 1.7);

player_t::player_t(shader_A_t &shader, world_buffer_t &world_buffer)
	:shader{shader}
	,world_buffer{world_buffer}
{  }

void player_t::init() {
	// Generate OpenGL ids
	texture_id = load_texture("runtime/player.png");

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

void player_t::deinit() {
	glDeleteBuffers(1,  &positions_buffer_id);
	glDeleteBuffers(1,  &uvs_buffer_id);
	glDeleteBuffers(1,  &normals_buffer_id);
	glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteTextures(1, &texture_id);

	glDeleteVertexArrays(1, &vao_id);
}

void player_t::draw(
	const glm::vec3 &light_pos,
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix
	) {
	// Shader
	glUseProgram(shader.program_id);

	// Set uniforms
	glm::mat4 model_matrix(1);
	model_matrix = glm::translate(
			model_matrix,
			position + glm::vec3((1.0f-hitbox_dimensions.x)/-2.0f, 0, 0)
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

#define SPEED 3
void player_t::move_up(float delta_time) {
	// move_by({0, SPEED * delta_time});
	on_axis_move_by(SPEED * delta_time, &glm::vec3::y);
}

void player_t::move_down(float delta_time) {
	// move_by({0, -SPEED * delta_time});
	on_axis_move_by(-SPEED * delta_time, &glm::vec3::y);
}

void player_t::move_right(float delta_time) {
	// move_by({-SPEED * delta_time, 0});
	on_axis_move_by(-SPEED * delta_time, &glm::vec3::x);
}

void player_t::move_left(float delta_time) {
	// move_by({SPEED * delta_time, 0});
	on_axis_move_by(SPEED * delta_time, &glm::vec3::x);
}

void player_t::jump(float delta_time) {
}
#undef SPEED

void player_t::on_axis_move_by(float offset, float glm::vec3::* axis_ptr) {
	if (offset == 0)
		return;

	float constrained_offset;
	float free_offset;
	float constrained_pos_comp; // Constrained position's component

	float colliding_point_pos_comp = position.*axis_ptr;
	float hitbox_dim_comp =
		glm::vec3(hitbox_dimensions.x, hitbox_dimensions.y, 0).*axis_ptr;
	if (offset > 0) {
		colliding_point_pos_comp += hitbox_dim_comp;

		// TODO: Distract an epsilon from this value?
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
		return;
	}

	glm::vec3 free_pos = position;
	free_pos.*axis_ptr += free_offset;
	if (world_buffer.collision_check_XY_rect(free_pos, hitbox_dimensions)) {
		position.*axis_ptr = constrained_pos_comp;
	} else {
		position.*axis_ptr += free_offset;
	}
}

// TODO: Rewrite this function so it breaks input vector into many short ones
// so it is possible to enter small gaps even with low framerate
// TODO: Break each XY vector into X and Y vectors and handle them with
// on_axis_move_by() function one after another
// Both changes won't give deterministic results, but will hopefully be good
// enough
glm::vec2 player_t::move_by(glm::vec2 offset) {
	if (offset == glm::vec2(0, 0))
		return offset;

#ifdef DEBUG
	const glm::vec3 initial_pos = position;
#endif

	const glm::vec2 lbc_pos
		(position.x+1.0f, position.y); // Left-bottom corner position

	glm::vec2 output(0.0f, 0.0f);

	while (offset != glm::vec2(0, 0)) {

		glm::vec2 vec_constrained_x; // Closest potential collision down axis
		glm::vec2 vec_constrained_y;
		float position_constrained_single_component_x;
		float position_constrained_single_component_y;
		if (offset.x > 0) {
			vec_constrained_x.x
				= std::ceil(lbc_pos.x) - lbc_pos.x;
			position_constrained_single_component_x
				= std::ceil(lbc_pos.x)-1.0f;
		} else {
			vec_constrained_x.x
				= std::floor(lbc_pos.x) - lbc_pos.x;
			position_constrained_single_component_x
				= std::floor(lbc_pos.x)-1.0f;
		}
		if (offset.y > 0) {
			vec_constrained_y.y
				= std::ceil(lbc_pos.y) - lbc_pos.y;
			position_constrained_single_component_y
				= std::ceil(lbc_pos.y) + 0;
		} else {
			vec_constrained_y.y
				= std::floor(lbc_pos.y) - lbc_pos.y;
			position_constrained_single_component_y
				= std::floor(lbc_pos.y) + 0;
		}
		vec_constrained_x.y = offset.y * vec_constrained_x.x / offset.x;
		vec_constrained_y.x = offset.x * vec_constrained_y.y / offset.y;

		glm::vec2 vec_constrained;
		glm::vec3 position_constrained;
		glm::vec2 vec;
		if (offset.y == 0.0f
				|| len_sq(vec_constrained_x) < len_sq(vec_constrained_y)) {
			vec_constrained = vec_constrained_x;
			position_constrained = glm::vec3(
					position_constrained_single_component_x,
					position.y + vec_constrained.y,
					position.z
					);

			if (offset.x > 0) {
				vec.x = vec_constrained.x + 1.0f;
				vec.x = std::min(vec.x, offset.x);
				if (vec.x <= vec_constrained.x) {
					output += offset;
					position += vec2_to_vec3(offset);
					return output;
				}
			} else {
				vec.x = vec_constrained.x - 1.0f;
				vec.x = std::max(vec.x, offset.x);
				if (vec.x >= vec_constrained.x) {
					output += offset;
					position += vec2_to_vec3(offset);
					return output;
				}
			}
			vec.y = offset.y * vec.x / offset.x;
		} else {
			vec_constrained = vec_constrained_y;
			position_constrained = glm::vec3(
					position.x + vec_constrained.x,
					position_constrained_single_component_y,
					position.z
					);

			if (offset.y > 0) {
				vec.y = vec_constrained.y + 1.0f;
				vec.y = std::min(vec.y, offset.y);
				if (vec.y <= vec_constrained.y) {
					output += offset;
					position += vec2_to_vec3(offset);
					return output;
				}
			} else {
				vec.y = vec_constrained.y - 1.0f;
				vec.y = std::max(vec.y, offset.y);
				if (vec.y >= vec_constrained.y) {
					output += offset;
					position += vec2_to_vec3(offset);
					return output;
				}
			}
			vec.x = offset.x * vec.y / offset.y;
		}

		const glm::vec2 new_lbc_pos = lbc_pos + vec;
		glm::ivec3 block_pos(
				static_cast<int>(std::floor(new_lbc_pos.x))-1,
				static_cast<int>(std::floor(new_lbc_pos.y)),
				static_cast<int>(std::floor(position.z))
				);

		for (int dx = 0; dx < 2; ++dx) {
			if (dx == 1 && std::floor(new_lbc_pos.x) == new_lbc_pos.x)
				break;

			for (int dy = 0; dy < 3; ++dy) {
				if (dy == 2 && std::floor(new_lbc_pos.y) == new_lbc_pos.y)
					break;

				if (world_buffer.get(
							block_pos+glm::ivec3(dx, dy, 0)
							) != block_type::none) {
					output += vec_constrained;
					position = position_constrained;
					return output;
				}
			}
		}

		output += vec;
		position += vec2_to_vec3(vec);
		offset -= vec;
	}

	return output;
}

void player_t::update_physics(float delta_time) {
}
