#include "generator.hpp"

#include "settings.hpp"
#include "useful.hpp"
#include "shader_loader.hpp"

#include <chrono>

void generator_C_t::init() {
#ifdef GENERATE_WITH_GPU
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
	t_uniform = glGetUniformLocation(program1, "t");
	space_max_uniform = glGetUniformLocation(program2, "space_max");
	glUseProgram(program2);
	glUniform2f(space_max_uniform, space_max.x, space_max.y);

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
	// static constexpr float vertices_pos[] {
	// 	-0.5, -0.5,
	// 	0.0,  0.5,
	// 	0.5, -0.5,
	// };
	// static constexpr uint8_t types[] {
	// 	2
	// };
	glGenBuffers(1, &continents_tiangles_pos_buf);
	glGenBuffers(1, &continents_type_buf);

	glBindBuffer(GL_ARRAY_BUFFER, continents_tiangles_pos_buf);
	// glBufferData(GL_ARRAY_BUFFER,
	// 		sizeof(vertices_pos),
	// 		vertices_pos,
	// 		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, continents_type_buf);
	// glBufferData(GL_ARRAY_BUFFER,
	// 		sizeof(types),
	// 		types,
	// 		GL_STATIC_DRAW);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
#endif
}

void generator_C_t::deinit() {
#ifdef GENERATE_WITH_GPU
	glDeleteProgram(program1);
	glDeleteProgram(program2);
	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &continents_vao);
#endif
}


void generator_C_t::draw_map_gpu() {
#ifdef GENERATE_WITH_GPU
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

	glUniform1f(t_uniform, t);

	// Execute shader
	glDispatchCompute(bitmap->WIDTH/3, bitmap->HEIGHT, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	// glFinish();

#elif PROG == 2
	// Prepare buffers
	continents_triangles_pos.resize(0);
	continents_type.resize(0);

	// continents_triangles_pos.push_back({-0.5, -0.5});
	// continents_triangles_pos.push_back({0.0,  0.5});
	// continents_triangles_pos.push_back({0.5, -0.5});
	// continents_type.push_back(2);

	for (size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		const voronoi_t &voro = diagram.voronois[i];
		const plate_t &plate = plates[i];
		for (const voronoi_t::edge_t &e : voro.al) {
			glm::dvec2 A = voro.center;
			glm::dvec2 B = e.end;
			glm::dvec2 C = e.beg;
			// A.x -= space_max.x / 3.0;
			// B.x -= space_max.x / 3.0;
			// C.x -= space_max.x / 3.0;
			// A /= space_max;
			// B /= space_max;
			// C /= space_max;
			// A = 2.0*A-1.0;
			// B = 2.0*B-1.0;
			// C = 2.0*C-1.0;
			continents_triangles_pos.push_back(A);
			continents_triangles_pos.push_back(B);
			continents_triangles_pos.push_back(C);
			continents_type.push_back(plate.type);
			continents_type.push_back(plate_t::NONE);
			continents_type.push_back(plate_t::NONE);
		}
	}

	// static constexpr float vertices_pos[] {
	// 	-0.5, -0.5,
	// 	0.0,  0.5,
	// 	0.5, -0.5,
	// };
	// static constexpr uint8_t types[] {
	// 	2
	// };
	// continents_triangles_pos.resize(sizeof(vertices_pos)/sizeof(float));
	// continents_type.resize(sizeof(types)/sizeof(uint8_t));
	// // continents_triangles_pos = vertices_pos;
	// // continents_type = types;
	glBindBuffer(GL_ARRAY_BUFFER, continents_tiangles_pos_buf);
	glBufferData(GL_ARRAY_BUFFER,
		continents_triangles_pos.size()*sizeof(continents_triangles_pos[0]),
		&continents_triangles_pos[0],
		// sizeof(vertices_pos),
		// vertices_pos,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, continents_type_buf);
	glBufferData(GL_ARRAY_BUFFER,
		continents_type.size()*sizeof(continents_type[0]),
		&continents_type[0],
		// sizeof(types),
		// types,
		GL_STATIC_DRAW);

	// Render
	glUseProgram(program2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindVertexArray(continents_vao);

	glViewport(0, 0, width, height);
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
	static const GLfloat blue[] = { 0.0f, 0.0f, 0.3f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, blue);

	// glDrawArrays(GL_TRIANGLES, 0, continents_type.size());
	glDrawArraysInstanced(GL_TRIANGLES, 0, continents_triangles_pos.size(), 3);
	// glDrawArrays(GL_TRIANGLES, 0, continents_triangles_pos.size());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	PRINT_ZU(continents_triangles_pos.size()/3);
	GL_GET_ERROR;
#endif
#endif
}
