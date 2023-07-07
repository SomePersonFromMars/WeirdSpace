#include "chunk.hpp"

#include <cstdio>

#include <texture_loader.hpp>
#include <useful.hpp>

#include "settings.hpp"

shader_world_t *chunk_t::pshader;
GLuint chunk_t::texture_id;
GLuint chunk_t::block_model_uniform_buffer_id;

void chunk_t::init_static(shader_world_t *shader_ptr) {
	chunk_t::pshader = shader_ptr;

	// Load the combined texture
	texture_id = load_texture(TEXTURE_BLOCKS_COMBINED_PATH);

	// Generate, fill and bind the uniform buffer object
	glGenBuffers(1, &block_model_uniform_buffer_id);
	glBindBuffer(GL_UNIFORM_BUFFER, block_model_uniform_buffer_id);
	// Allocate the buffer
	glBufferData(GL_UNIFORM_BUFFER,
		sizeof(BLOCK_POSITIONS)+sizeof(BLOCK_UVS)+sizeof(BLOCK_NORMALS),
		nullptr, GL_STATIC_DRAW);

	// Fill the buffer
	glBufferSubData(GL_UNIFORM_BUFFER,
			0, sizeof(BLOCK_POSITIONS), &BLOCK_POSITIONS[0]);
	glBufferSubData(GL_UNIFORM_BUFFER,
			sizeof(BLOCK_POSITIONS), sizeof(BLOCK_UVS), &BLOCK_UVS[0]);
	glBufferSubData(GL_UNIFORM_BUFFER,
			sizeof(BLOCK_POSITIONS)+sizeof(BLOCK_UVS), sizeof(BLOCK_NORMALS),
			&BLOCK_NORMALS[0]);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the whole buffer to GL_UNIFORM_BUFFER indexed buffer
	// at index shader.block_model_uniform_binding_point
	glBindBufferRange(GL_UNIFORM_BUFFER,
		pshader->block_model_uniform_binding_point,
		block_model_uniform_buffer_id, 0,
		sizeof(BLOCK_POSITIONS)+sizeof(BLOCK_UVS)+sizeof(BLOCK_NORMALS));
}

chunk_t::chunk_t()
{
	for (size_t x = 0; x < WIDTH; ++x)
		for (size_t y = 0; y < HEIGHT; ++y)
			for (size_t z = 0; z < DEPTH; ++z)
				content[x][y][z] = block_type::none;

	// VAO
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &positions_instanced_buffer_id);
	glGenBuffers(1, &blocks_types_instanced_buffer_id);
	glGenBuffers(1, &faces_types_instanced_buffer_id);

	// Initialize VBOs with single instance data
	// There is none

	// Add shader vertex attributes to the VAO
	// Single vertex attributes
	// There are none

	// Instanced data
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(0, 1);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, (void*)0);
	glVertexAttribDivisor(1, 1);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, faces_types_instanced_buffer_id);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 0, (void*)0);
	glVertexAttribDivisor(2, 1);

	glBindVertexArray(0);

	// Allocate instanced buffers
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
		chunk_t::WIDTH*chunk_t::HEIGHT*chunk_t::DEPTH*sizeof(GLfloat)*3,
		nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
		chunk_t::WIDTH*chunk_t::HEIGHT*chunk_t::DEPTH*sizeof(uint8_t),
		nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, faces_types_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
		chunk_t::WIDTH*chunk_t::HEIGHT*chunk_t::DEPTH*sizeof(uint8_t),
		nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void chunk_t::deinit_static() {
	glDeleteTextures(1, &texture_id);
	glDeleteBuffers(1, &block_model_uniform_buffer_id);
}

chunk_t::~chunk_t() {
	glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteBuffers(1,  &blocks_types_instanced_buffer_id);
	glDeleteBuffers(1,  &faces_types_instanced_buffer_id);

	glDeleteVertexArrays(1, &vao_id);
}

void chunk_t::clear_preprocessing_data() {
	positions_instanced_buffer.clear();
	blocks_types_instanced_buffer.clear();
	faces_types_instanced_buffer.clear();
}

void chunk_t::preprocess() {
	float average_faces_visible = 0;
	float visible_blocks_cnt = 0;

	for (size_t x = 0; x < WIDTH; ++x) {
		for (size_t y = 0; y < HEIGHT; ++y) {
			for (size_t z = 0; z < DEPTH; ++z) {
				if (content[x][y][z] == block_type::none) continue;

				uint8_t faces_mask = 0x3f;

				if (x > 0) {
					if (content[x-1][y][z] != block_type::none)
						faces_mask &= ~(1<<3);
				} else {
					if (neighbors[0] != nullptr
						&& neighbors[0]->content[WIDTH-1][y][z]
						!= block_type::none)
						faces_mask &= ~(1<<3);
				}
				if (x < WIDTH-1) {
					if (content[x+1][y][z] != block_type::none)
						faces_mask &= ~(1<<2);
				} else {
					if (neighbors[1] != nullptr
						&& neighbors[1]->content[0][y][z]
						!= block_type::none)
						faces_mask &= ~(1<<2);
				}

				if (y > 0) {
					if (content[x][y-1][z] != block_type::none)
						faces_mask &= ~(1<<4);
				} else {
					if (neighbors[2] != nullptr
						&& neighbors[2]->content[x][HEIGHT-1][z]
						!= block_type::none)
						faces_mask &= ~(1<<4);
				}
				if (y < HEIGHT-1) {
					if (content[x][y+1][z] != block_type::none)
						faces_mask &= ~(1<<1);
				} else {
					if (neighbors[3] != nullptr
						&& neighbors[3]->content[x][0][z]
						!= block_type::none)
						faces_mask &= ~(1<<1);
				}

				if (z > 0) {
					if (content[x][y][z-1] != block_type::none)
						faces_mask &= ~(1<<0);
				} else {
					if (neighbors[4] != nullptr
						&& neighbors[4]->content[x][y][DEPTH-1]
						!= block_type::none)
						faces_mask &= ~(1<<0);
				}
				if (z < DEPTH-1) {
					if (content[x][y][z+1] != block_type::none)
						faces_mask &= ~(1<<5);
				} else {
					if (neighbors[5] != nullptr
						&& neighbors[5]->content[x][y][0]
						!= block_type::none)
						faces_mask &= ~(1<<5);
				}

				if (faces_mask == 0)
					continue;

				average_faces_visible += __builtin_popcount(faces_mask);
				// average_faces_visible += 6;
				visible_blocks_cnt += 1;

				const glm::vec3 block_pos(
					static_cast<float>(x),
					static_cast<float>(y),
					static_cast<float>(z)
				);

				for (uint8_t i = 0; i < 6; ++i) {
					if (!(faces_mask & (1<<i)))
						continue;

					positions_instanced_buffer.push_back(block_pos.x);
					positions_instanced_buffer.push_back(block_pos.y);
					positions_instanced_buffer.push_back(block_pos.z);

					blocks_types_instanced_buffer.push_back(
						static_cast<uint8_t>(content[x][y][z]));

					faces_types_instanced_buffer.push_back(i);
				}
			}
		}
	}

	average_faces_visible /= visible_blocks_cnt;
	PRINT_F(average_faces_visible);
	PRINT_F(visible_blocks_cnt);
}

void chunk_t::send_preprocessed_to_gpu() {
	printf("%zu\n", positions_instanced_buffer.size());
	printf("%zu\n", blocks_types_instanced_buffer.size());
	printf("%zu\n", faces_types_instanced_buffer.size());

	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
			positions_instanced_buffer.size()*sizeof(GLfloat),
			&positions_instanced_buffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
			blocks_types_instanced_buffer.size()*sizeof(GLubyte),
			&blocks_types_instanced_buffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, faces_types_instanced_buffer_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
			faces_types_instanced_buffer.size()*sizeof(GLubyte),
			&faces_types_instanced_buffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void chunk_t::draw(
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix,
	const glm::mat4 &model_matrix,
	const glm::vec3 &light_pos
	) {
	glUseProgram(pshader->program_id);

	glUniformMatrix4fv(pshader->model_matrix_uniform,
		1, GL_FALSE, &model_matrix[0][0]);
	glUniformMatrix4fv(pshader->view_matrix_uniform,
		1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(pshader->projection_matrix_uniform,
		1, GL_FALSE, &projection_matrix[0][0]);

	const glm::vec3 light_color = color_hex_to_vec3(LIGHT_COLOR);
	glUniform3f(pshader->light_pos_worldspace_uniform,
		light_pos.x, light_pos.y, light_pos.z);
	glUniform3f(pshader->light_color_uniform,
		light_color.x, light_color.y, light_color.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(pshader->texture_sampler_uniform,
			0);

	glBindVertexArray(vao_id);
	glDrawArraysInstanced(GL_TRIANGLES,
		0, 6,
		positions_instanced_buffer.size()/3
	);
	glBindVertexArray(0);
}
