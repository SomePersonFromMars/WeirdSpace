#include "generator.hpp"

#include "settings.hpp"
#include "useful.hpp"
#include "shader_loader.hpp"

#include <chrono>

void generator_C_t::init() {
	// Load shaders and generate shader programs
	GLuint compute_shader_id = compile_shader(
			SHADER_GENERATOR_D_COMPUTE_PATH, GL_COMPUTE_SHADER);
	program1 = link_program(1, compute_shader_id);
	delete_shader(compute_shader_id);

	GLuint vertex_shader_id = compile_shader(
			SHADER_GENERATOR_D_VERRTEX_PATH, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = compile_shader(
			SHADER_GENERATOR_D_FRAGMENT_PATH, GL_FRAGMENT_SHADER);
	program2 = link_program(2, vertex_shader_id, fragment_shader_id);
	delete_shader(vertex_shader_id);
	delete_shader(fragment_shader_id);

	// Uniforms
	t_prog1_uniform = glGetUniformLocation(program1, "t");
	t_prog2_uniform = glGetUniformLocation(program1, "t");
	space_max_uniform = glGetUniformLocation(program2, "space_max");

	// Create FBO etc.
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			bitmap->get_texture_id(), 0);
	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Continents vao etc.
	glGenVertexArrays(1, &continents_vao);
	glBindVertexArray(continents_vao);

	// Buffers
	glGenBuffers(1, &continents_triangle_pos_buf);
	glGenBuffers(1, &continents_elevation_buf);
	// glGenBuffers(1, &continents_type_buf);

	glBindBuffer(GL_ARRAY_BUFFER, continents_triangle_pos_buf);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, continents_elevation_buf);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	// glBindBuffer(GL_ARRAY_BUFFER, continents_type_buf);
	// glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, NULL);
	// glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	load_settings();
}

void generator_C_t::deinit() {
	glDeleteProgram(program1);
	glDeleteProgram(program2);
	glDeleteBuffers(1, &continents_triangle_pos_buf);
	glDeleteBuffers(1, &continents_elevation_buf);
	// glDeleteBuffers(1, &continents_type_buf);
	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &continents_vao);
}


void generator_C_t::draw_map_gpu() {
#define PROG 2
#if PROG == 1
	// Shader program
	glUseProgram(program1);

	// Uniforms and texture
	glBindImageTexture(
			0,
			bitmap->get_texture_id(),
			0,
			GL_FALSE,
			0,
			GL_WRITE_ONLY,
			GL_RGBA8
			);

	const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();
	static auto app_start_ms = now_ms;
	const auto elapsed_ms = now_ms - app_start_ms;
	const float t = float(elapsed_ms);
	// PRINT_F(t);

	glUniform1f(t_prog1_uniform, t);

	// Execute shader
	glDispatchCompute(bitmap->width/3, bitmap->height, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	// glFinish();

#elif PROG == 2
	// Prepare buffers
	continents_triangles_pos.resize(0);
	continents_elevation.resize(0);
	// continents_type.resize(0);

	for (size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		const voronoi_t &voro = diagram.voronois[i];
		const plate_t &plate = plates[i];
		for (size_t j = 0; j < voro.al.size(); ++j) {
			const voronoi_t::edge_t &e = voro.al[j];
			const plate_t::type_t type = plate.type;
			assert(type != plate_t::NONE);
			const plate_t::type_t other_type
				= type==plate_t::LAND ? plate_t::WATER : plate_t::LAND;
			const float elevation = type==plate_t::LAND ? 1.0 : 0.0;
			const float other_elevation = 0.5;
			if (type != plate_t::NONE) {
				if (plates[e.neighbor_id].type == other_type) {
					continents_triangles_pos.push_back(voro.center);
					continents_triangles_pos.push_back(e.end);
					continents_triangles_pos.push_back(e.beg);
					continents_elevation.push_back(elevation);
					continents_elevation.push_back(other_elevation);
					continents_elevation.push_back(other_elevation);
					// continents_type.push_back(type);
					// continents_type.push_back(other_type);
					// continents_type.push_back(other_type);
				} else if (plates[e.neighbor_id].type == type) {
					size_t prev = j;
					size_t nxt = j;
					if (prev != 0) --prev;
					else if (voro.clipped) prev = INVALID_ID;
					else prev = voro.al.size()-1;
					if (nxt != voro.al.size()-1) ++nxt;
					else if (voro.clipped) nxt = INVALID_ID;
					else nxt = 0;
					plate_t::type_t prev_type = plate_t::NONE;
					plate_t::type_t nxt_type  = plate_t::NONE;
					if (prev != INVALID_ID)
						prev_type = plates[voro.al[prev].neighbor_id].type;
					if (nxt != INVALID_ID)
						nxt_type = plates[voro.al[nxt].neighbor_id].type;

					const glm::dvec2 mid = (e.beg + e.end) / 2.0;
					if (prev_type != plate_t::NONE) {
						continents_triangles_pos.push_back(voro.center);
						continents_triangles_pos.push_back(mid);
						continents_triangles_pos.push_back(e.beg);
						if (prev_type == type) {
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(elevation);
							// continents_type.push_back(type);
							// continents_type.push_back(type);
							// continents_type.push_back(type);
						} else if (prev_type == other_type) {
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(other_elevation);
							// continents_type.push_back(type);
							// continents_type.push_back(type);
							// continents_type.push_back(other_type);
						} else {
							assert(false);
						}
					}
					if (nxt_type != plate_t::NONE) {
						continents_triangles_pos.push_back(voro.center);
						continents_triangles_pos.push_back(e.end);
						continents_triangles_pos.push_back(mid);
						if (nxt_type == type) {
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(elevation);
							// continents_type.push_back(type);
							// continents_type.push_back(type);
							// continents_type.push_back(type);
						} else if (nxt_type == other_type) {
							continents_elevation.push_back(elevation);
							continents_elevation.push_back(other_elevation);
							continents_elevation.push_back(elevation);
							// continents_type.push_back(type);
							// continents_type.push_back(other_type);
							// continents_type.push_back(type);
						} else {
							assert(false);
						}
					}
				} else {
					assert(false);
				}
			} else {
				continents_triangles_pos.push_back(voro.center);
				continents_triangles_pos.push_back(e.end);
				continents_triangles_pos.push_back(e.beg);
				// if (type == plate_t::WATER) {
				// 	continents_type.push_back(plate_t::WATER);
				// 	continents_type.push_back(plate_t::WATER);
				// 	continents_type.push_back(plate_t::WATER);
				// } else {
				// }
				continents_elevation.push_back(0.0);
				continents_elevation.push_back(0.0);
				continents_elevation.push_back(0.0);
				// continents_type.push_back(plate_t::NONE);
				// continents_type.push_back(plate_t::NONE);
				// continents_type.push_back(plate_t::NONE);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, continents_triangle_pos_buf);
	glBufferData(GL_ARRAY_BUFFER,
		continents_triangles_pos.size()*sizeof(continents_triangles_pos[0]),
		&continents_triangles_pos[0],
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, continents_elevation_buf);
	glBufferData(GL_ARRAY_BUFFER,
		continents_elevation.size()*sizeof(continents_elevation[0]),
		&continents_elevation[0],
		GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, continents_type_buf);
	// glBufferData(GL_ARRAY_BUFFER,
	// 	continents_type.size()*sizeof(continents_type[0]),
	// 	&continents_type[0],
	// 	GL_STATIC_DRAW);

	// Render
	glUseProgram(program2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindVertexArray(continents_vao);

	glViewport(0, 0, width, height);
	static const GLfloat blue[] = { 0.0f, 0.0f, 0.3f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, blue);

	const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()
			).count();
	static auto app_start_ms = now_ms;
	const auto elapsed_ms = now_ms - app_start_ms;
	const float t = float(elapsed_ms);
	glUniform1f(t_prog2_uniform, t);

	glDrawArraysInstanced(GL_TRIANGLES, 0, continents_triangles_pos.size(), 3);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	PRINT_ZU(continents_triangles_pos.size()/3);
	GL_GET_ERROR;
#endif
}
