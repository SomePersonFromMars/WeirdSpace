#include "world_renderer.hpp"

#include <cstdio>

#include <glm/gtc/matrix_transform.hpp>
#include "texture_loader.hpp"

world_renderer_t::world_renderer_t(
		shader_world_B_t &shader,
		world_buffer_t &buffer
	)
	:shader{shader}
	,buffer{buffer}
{  }

void world_renderer_t::init() {
	// Generate OpenGL ids
	texture_id = load_texture(TEXTURE_BLOCKS_COMBINED_PATH);

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &positions_buffer_id);
	glGenBuffers(1, &uvs_buffer_id);
	glGenBuffers(1, &normals_buffer_id);

	glGenBuffers(1, &positions_instanced_buffer_id);
	glGenBuffers(1, &blocks_types_instanced_buffer_id);
	glGenBuffers(1, &faces_masks_instanced_buffer_id);

	// Initialize VBOs with single instance data
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(single_block_positions),
			single_block_positions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(single_block_uvs),
			single_block_uvs, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(single_block_normals),
			single_block_normals, GL_STATIC_DRAW);

	// Add shader vertex attributes to the VAO
	// Single instance vertex attributes
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, positions_buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer_id);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_buffer_id);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Instanced data
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(3, 1);

	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, 0, (void*)0);
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(5);
	glBindBuffer(GL_ARRAY_BUFFER, faces_masks_instanced_buffer_id);
	glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, 0, (void*)0);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);

	// Allocate instanced buffer
	// TODO: The buffer size is too small for more than one chunk
	// so the world_buffer_t functionality has to be moved to the
	// chunk_t class.
	glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			chunk_t::WIDTH*chunk_t::HEIGHT*chunk_t::DEPTH*sizeof(GLfloat),
			nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			chunk_t::WIDTH*chunk_t::HEIGHT*chunk_t::DEPTH*sizeof(uint8_t),
			nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, faces_masks_instanced_buffer_id);
	glBufferData(GL_ARRAY_BUFFER,
			chunk_t::WIDTH*chunk_t::HEIGHT*chunk_t::DEPTH*sizeof(uint8_t),
			nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void world_renderer_t::clear_preprocessing_data() {
	positions_instanced_buffer.clear();
	blocks_types_instanced_buffer.clear();
	faces_masks_instanced_buffer.clear();
}

// TODO: While instanced drawing iterate over FACES instead of CUBES,
// because now the drawing performance bottleneck is too many vertex
// shader's instances being CALLED. It should be much faster since
// pessimistically ~1.5 faces are visible per cube on average. Right now when
// you constrain the glDrawArraysInstanced with only 12 vertices instead of 36
// you get framerate 40-60 fps on an integrated GPU compared to 20-30 fps.
void world_renderer_t::preprocess_chunk(const glm::ivec2 &chunk_pos) {
	const auto &chunk = buffer.chunks[chunk_pos];
	const auto &content = chunk.content;
	const glm::vec2 offset(
		chunk_pos.x * static_cast<float>(chunk_t::WIDTH),
		chunk_pos.y * static_cast<float>(chunk_t::DEPTH)
	);

	float average_faces_visible = 0;
	float visible_blocks_cnt = 0;

	for (size_t x = 0; x < chunk.WIDTH; ++x) {
		for (size_t y = 0; y < chunk.HEIGHT; ++y) {
			for (size_t z = 0; z < chunk.DEPTH; ++z) {
				if (content[x][y][z] == block_type::none) continue;

				uint8_t faces_mask = 0x3f;
				if (x > 0 && content[x-1][y][z] != block_type::none)
					faces_mask &= ~(1<<3);
				if (x < chunk.WIDTH-1 && content[x+1][y][z] != block_type::none)
					faces_mask &= ~(1<<2);
				if (y > 0 && content[x][y-1][z] != block_type::none)
					faces_mask &= ~(1<<4);
				if (y < chunk.WIDTH-1 && content[x][y+1][z] != block_type::none)
					faces_mask &= ~(1<<1);
				if (z > 0 && content[x][y][z-1] != block_type::none)
					faces_mask &= ~(1<<0);
				if (z < chunk.WIDTH-1 && content[x][y][z+1] != block_type::none)
					faces_mask &= ~(1<<5);

				if (faces_mask == 0)
					continue;

				average_faces_visible += __builtin_popcount(faces_mask);
				// average_faces_visible += 6;
				visible_blocks_cnt += 1;

				// if (x > 0 && x < chunk.WIDTH-1
				// 	&& y > 0 && y < chunk.HEIGHT-1
				// 	&& z > 0 && z < chunk.DEPTH-1
				// 	&& content[x-1][y][z] != block_type::none
				// 	&& content[x+1][y][z] != block_type::none
				// 	&& content[x][y-1][z] != block_type::none
				// 	&& content[x][y+1][z] != block_type::none
				// 	&& content[x][y][z-1] != block_type::none
				// 	&& content[x][y][z+1] != block_type::none
				// ) continue;

				const glm::vec3 block_pos(
					static_cast<float>(x) + offset.x,
					static_cast<float>(y),
					static_cast<float>(z) + offset.y
				);

				// positions_instanced_buffer.resize(
				// 		positions_instanced_buffer.size()+3);
				// positions_instanced_buffer
				// 	[positions_instanced_buffer.size()-3] = block_pos.x;
				// positions_instanced_buffer
				// 	[positions_instanced_buffer.size()-2] = block_pos.y;
				// positions_instanced_buffer
				// 	[positions_instanced_buffer.size()-1] = block_pos.z;
				positions_instanced_buffer.push_back(block_pos.x);
				positions_instanced_buffer.push_back(block_pos.y);
				positions_instanced_buffer.push_back(block_pos.z);

				// blocks_types_instanced_buffer.push_back(content[x][y][z]);
				blocks_types_instanced_buffer.push_back(1);

				faces_masks_instanced_buffer.push_back(faces_mask);
				// faces_masks_instanced_buffer.push_back(0x3f);
				// faces_masks_instanced_buffer.push_back(0x3);
			}
		}
	}

	average_faces_visible /= visible_blocks_cnt;
	PRINT_F(average_faces_visible);
	PRINT_F(visible_blocks_cnt);
}

void world_renderer_t::finish_preprocessing() {
	// glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
	// glBufferData(GL_ARRAY_BUFFER,
	// 		positions_instanced_buffer.size()*sizeof(GLfloat),
	// 		&positions_instanced_buffer[0], GL_STATIC_DRAW);

	printf("%zu\n", positions_instanced_buffer.size());
	printf("%zu\n", blocks_types_instanced_buffer.size());
	printf("%zu\n", faces_masks_instanced_buffer.size());

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

	glBindBuffer(GL_ARRAY_BUFFER, faces_masks_instanced_buffer_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
			faces_masks_instanced_buffer.size()*sizeof(GLubyte),
			&faces_masks_instanced_buffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void world_renderer_t::draw(
	const glm::vec3 &light_pos,
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix,
	const glm::mat4 &model_matrix
) {
	glUseProgram(shader.program_id);

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

	glBindVertexArray(vao_id);
	glDrawArraysInstanced(GL_TRIANGLES,
		0, single_block_points_cnt,
		// 0, 12,
		positions_instanced_buffer.size()/3
	);
	// glDrawArraysInstanced(GL_TRIANGLES,
	// 	// 0, sizeof(single_block_positions)/sizeof(GLfloat)/3,
	// 	0, 36,
	// 	positions_instanced_buffer.size()/3
	// );
}

void world_renderer_t::deinit() {
	glDeleteBuffers(1,  &positions_buffer_id);
	glDeleteBuffers(1,  &uvs_buffer_id);
	glDeleteBuffers(1,  &normals_buffer_id);
	glDeleteBuffers(1,  &positions_instanced_buffer_id);
	glDeleteBuffers(1,  &blocks_types_instanced_buffer_id);
	glDeleteBuffers(1,  &faces_masks_instanced_buffer_id);
	glDeleteTextures(1, &texture_id);

	glDeleteVertexArrays(1, &vao_id);
}
