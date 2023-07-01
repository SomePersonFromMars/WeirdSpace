#pragma once
#ifndef APP_HPP
#define APP_HPP

#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <imgui.h>

#include "generator.hpp"
#include "line.hpp"

struct callbacks_strct_t {
	callbacks_strct_t(GLint &window_width, GLint &window_height);
	GLint &window_width, &window_height;
	generator_C_t *generator_C = nullptr;
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
	void init_generator();

	void loop_input();
	void loop_imgui();
	void loop_generator();

	void deinit_imgui();

	void soft_reload_procedure();
	void reload_procedure();

	GLFWwindow* window;
	GLint window_width = 1080*2;
	GLint window_height = 1080;
	callbacks_strct_t callbacks_strct;

	const glm::vec3 background_color = glm::vec3(0.0);
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bitmap_t bitmap_A;
	generator_C_t generator_C;
	line_t line;
	double line_off = 0;
	glm::vec3 camera_pos = {0, 0, 0};
	float camera_zoom = 1;

	std::chrono::time_point<std::chrono::high_resolution_clock> timer_logging;
	double timer_fps_cnter;
	double delta_time;

	glm::mat4 MVPb;
	glm::vec3 mp;
};

#endif
