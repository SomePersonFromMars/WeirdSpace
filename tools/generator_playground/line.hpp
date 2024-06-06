// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef LINE_HPP
#define LINE_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class line_t {
public:
	line_t() = default;
	void init();
	~line_t();
	void draw(const glm::mat4 &MVP_matrix);

private:
	GLuint program_id;
	GLuint vao_id;
	GLuint quad_positions_buffer_id;
	GLuint MVP_matrix_uniform;

	static constexpr GLfloat quad_positions[] {
		-1, -1, 0,
		-1, 1, 0,
		1, -1, 0,
		1, 1, 0,
	};
};

#endif
