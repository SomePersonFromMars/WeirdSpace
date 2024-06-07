// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef APP_HPP
#define APP_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "map_generator/map_generator.hpp"
#include "line.hpp"

struct callbacks_strct_t {
	callbacks_strct_t(GLint &window_width, GLint &window_height);
	GLint &window_width, &window_height;
	map_generator_t *map_generator = nullptr;
	bool refresh_required = false;

	static inline callbacks_strct_t* get_strct(GLFWwindow *window) {
		return reinterpret_cast<callbacks_strct_t*>(
			glfwGetWindowUserPointer(window)
		);
	}
};

class app_t {
public:
	app_t();

	void init();
	void loop();
	void deinit();

private:
	void init_opengl_etc();
	void init_imgui();
	void init_map_generator();

	void in_loop_parse_input();
	void in_loop_update_imgui();
	void in_loop_draw_map();

	void deinit_opengl_etc();
	void deinit_imgui();
	void deinit_map_generator();

	void draw_playground_specific_imgui_widgets();
    void draw_playground_instructions();
	void soft_reload_procedure();
	void reload_procedure();

	GLFWwindow* window;
	GLint window_width = 1080*2;
	GLint window_height = 1080;
	callbacks_strct_t callbacks_strct;

	map_storage_t map_storage;
	map_generator_t map_generator;
	line_t line;
	double line_off = 0;
	glm::vec3 camera_pos = {0, 0, 0};
	float camera_zoom = 1;

	double delta_time;

	glm::mat4 MVPb;
	glm::vec3 mp;
};

#endif
