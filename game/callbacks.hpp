#pragma once
#ifndef CALLBACKS_HPP
#define CALLBACKS_HPP

#include <bitset>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "player.hpp"

void window_size_callback(GLFWwindow* window, int width, int height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window,
		int key, int scancode, int action, int mods);

struct callbacks_strct_t {
	callbacks_strct_t(
			GLFWwindow *window,
			GLint &window_width,
			GLint &window_height,
			double &delta_time,
			camera_t &camera,
			player_t &player
		);
	void handle_input();

private:
	GLint &window_width;
	GLint &window_height;
	double &delta_time;
	camera_t &camera;
	player_t &player;

	std::bitset<GLFW_KEY_LAST+1> key_holded;

	static inline callbacks_strct_t* get_strct(GLFWwindow *window);
	friend void window_size_callback
		(GLFWwindow* window, int width, int height);
	friend void framebuffer_size_callback
		(GLFWwindow* window, int width, int height);
	friend void key_callback(GLFWwindow *window,
			int key, int scancode, int action, int mods);
};
inline callbacks_strct_t* callbacks_strct_t::get_strct(
		GLFWwindow *window) {
	return reinterpret_cast<callbacks_strct_t*>(
			glfwGetWindowUserPointer(window)
		);
}

#endif
