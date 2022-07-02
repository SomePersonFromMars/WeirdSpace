#include "world_renderer.hpp"

world_renderer_t::world_renderer_t(world_buffer_t &buffer)
	:buffer{buffer}
{  }

void world_renderer_t::init() {
	program_id = LoadShaders( "vertex.vs", "fragment.fs" );
	MVP_matrix_uniform = glGetUniformLocation(program_id, "MVP");
	view_matrix_uniform = glGetUniformLocation(program_id, "V");
	model_matrix_uniform = glGetUniformLocation(program_id, "M");
	light_uniform = glGetUniformLocation(program_id,
			"LightPosition_worldspace");

	for (uint8_t i = 1; i < static_cast<uint8_t>(block_type::cnt); ++i) {
		strcts[i].texture_id = loadDDS("sand.dds");
		strcts[i].texture_uniform = glGetUniformLocation(program_id,
				"myTextureSampler");

		glGenBuffers(1, &strcts[i].positions_buffer_id);
		glGenBuffers(1, &strcts[i].uvs_buffer_id);
		glGenBuffers(1, &strcts[i].normals_buffer_id);
	}
}

void world_renderer_t::clear_preprocessing_data() {
	for (uint8_t i = 1; i < static_cast<uint8_t>(block_type::cnt); ++i) {
		strcts[i].positions_buffer.clear();
		strcts[i].uvs_buffer.clear();
		strcts[i].normals_buffer.clear();
	}
}

void world_renderer_t::preprocess_chunk(const glm::ivec2 &chunk_pos) {
	const auto &content = buffer.chunks[chunk_pos].content;
	const glm::vec2 offset(
		chunk_pos.x * static_cast<float>(chunk_t::width),
		chunk_pos.y * static_cast<float>(chunk_t::depth)
	);
	// const float x_offset = chunk_pos.x * static_cast<float>(chunk_t::width);

	for (size_t x = 0; x < content.size(); ++x) {
		for (size_t y = 0; y < content[x].size(); ++y) {
			for (size_t z = 0; z < content[x][y].size(); ++z) {
				if (content[x][y][z] == block_type::none) continue;
				auto &strct = strcts[static_cast<uint8_t>(content[x][y][z])];

				for (size_t i = 0; i < single_block_points_cnt; ++i) {
					const glm::vec3 pos(
						single_block_positions[3*i+0] + 2.0f * x,
						single_block_positions[3*i+1] + 2.0f * y,
						single_block_positions[3*i+2] + -2.0f * z
					);
					strct.positions_buffer.push_back(pos.x / 2.0f + offset.x);
					strct.positions_buffer.push_back(pos.y / 2.0f);
					strct.positions_buffer.push_back(pos.z / 2.0f + offset.y);

					const glm::vec2 uv(
						single_block_uv[2*i+0],
						single_block_uv[2*i+1]
					);
					strct.uvs_buffer.push_back(uv.x);
					strct.uvs_buffer.push_back(uv.y);

					const glm::vec3 normal(
						single_block_normals[3*i+0] + 2.0f * x,
						single_block_normals[3*i+1] + 2.0f * y,
						single_block_normals[3*i+2] + -2.0f * z
					);
					strct.normals_buffer.push_back(normal.x / 2.0f + offset.x);
					strct.normals_buffer.push_back(normal.y / 2.0f);
					strct.normals_buffer.push_back(normal.z / 2.0f + offset.y);
				}
			}
		}
	}
}

void world_renderer_t::finish_preprocessing() {
	for (uint8_t i = 1; i < static_cast<uint8_t>(block_type::cnt); ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, strcts[i].positions_buffer_id);
		glBufferData(GL_ARRAY_BUFFER,
				strcts[i].positions_buffer.size()*sizeof(GLfloat),
				&strcts[i].positions_buffer[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, strcts[i].uvs_buffer_id);
		glBufferData(GL_ARRAY_BUFFER,
				strcts[i].uvs_buffer.size()*sizeof(GLfloat),
				&strcts[i].uvs_buffer[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, strcts[i].normals_buffer_id);
		glBufferData(GL_ARRAY_BUFFER,
				strcts[i].normals_buffer.size()*sizeof(GLfloat),
				&strcts[i].normals_buffer[0], GL_STATIC_DRAW);
	}
}

void world_renderer_t::draw(
	const glm::vec3 &camera_pos,
	const glm::mat4 &projection_matrix,
	const glm::mat4 &view_matrix,
	const glm::mat4 &model_matrix
) {
	// Use our shader
	glUseProgram(program_id);

	glm::mat4 MVP_matrix = projection_matrix * view_matrix * model_matrix;

	glUniformMatrix4fv(MVP_matrix_uniform, 1, GL_FALSE, &MVP_matrix[0][0]);
	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, &model_matrix[0][0]);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, &view_matrix[0][0]);

	const glm::vec3 light_pos = glm::vec3(10, 20, -10) + camera_pos;
	glUniform3f(light_uniform, light_pos.x, light_pos.y, light_pos.z);

	for (uint8_t i = 1; i < static_cast<uint8_t>(block_type::cnt); ++i) {
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, strcts[i].texture_id);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(strcts[i].texture_uniform, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, strcts[i].positions_buffer_id);
		glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

		// 2nd attribute buffer : UV coordinates
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, strcts[i].uvs_buffer_id);
		glVertexAttribPointer(
				1,                  // attribute
				2,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, strcts[i].normals_buffer_id);
		glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

		glDrawArrays(GL_TRIANGLES, 0, strcts[i].positions_buffer.size()/3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}
}

void world_renderer_t::deinit() {
	for (uint8_t i = 1; i < static_cast<uint8_t>(block_type::cnt); ++i) {
		glDeleteBuffers(1,  &strcts[i].positions_buffer_id);
		glDeleteBuffers(1,  &strcts[i].uvs_buffer_id);
		glDeleteBuffers(1,  &strcts[i].normals_buffer_id);
		glDeleteTextures(1, &strcts[i].texture_id);
	}

	glDeleteProgram(program_id);
}
