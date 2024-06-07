// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "callbacks.hpp"
#include "GLFW/glfw3.h"

#include <useful.hpp>

callbacks_strct_t::callbacks_strct_t(
		GLint &window_width,
		GLint &window_height,
		double &delta_time,
		camera_t &camera,
		player_t &player
	)
	:window_width{window_width}
	,window_height{window_height}
	,delta_time{delta_time}
	,camera{camera}
	,player{player}
{ }

void callbacks_strct_t::init_gl(GLFWwindow *window) {
	glfwSetWindowUserPointer(
			window,
			reinterpret_cast<void*>(this)
		);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
}

void callbacks_strct_t::handle_input() {
	camera.enable_moving_acceleration(  key_holded[GLFW_KEY_LEFT_SHIFT] or key_holded[GLFW_KEY_SEMICOLON]);
	camera.enable_rotation_acceleration(key_holded[GLFW_KEY_LEFT_SHIFT] or key_holded[GLFW_KEY_SEMICOLON]);
	player.enable_moving_acceleration(key_holded[GLFW_KEY_Z]);

	if (key_holded[GLFW_KEY_KP_8] or key_holded[GLFW_KEY_I])
		camera.rotate_up(delta_time);

	if (key_holded[GLFW_KEY_KP_2] or key_holded[GLFW_KEY_K])
		camera.rotate_down(delta_time);

	if (key_holded[GLFW_KEY_KP_6] or key_holded[GLFW_KEY_L])
		camera.rotate_right(delta_time);

	if (key_holded[GLFW_KEY_KP_4] or key_holded[GLFW_KEY_J])
		camera.rotate_left(delta_time);


	if (key_holded[GLFW_KEY_W])
		camera.move_forward(delta_time);

	if (key_holded[GLFW_KEY_S])
		camera.move_backward(delta_time);

	if (key_holded[GLFW_KEY_D])
		camera.move_right(delta_time);

	if (key_holded[GLFW_KEY_A])
		camera.move_left(delta_time);


	if (key_holded[GLFW_KEY_UP])
		player.move_up(delta_time);

	if (key_holded[GLFW_KEY_DOWN])
		player.move_down(delta_time);

	if (key_holded[GLFW_KEY_RIGHT])
		player.move_right(delta_time);

	if (key_holded[GLFW_KEY_LEFT])
		player.move_left(delta_time);

	if (key_holded[GLFW_KEY_SPACE])
		player.jump(delta_time);
}

void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);
	strct_ptr->window_width = width;
	strct_ptr->window_height = height;
}

void key_callback(GLFWwindow *window,
		int key, [[maybe_unused]] int scancode,
		int action, [[maybe_unused]] int mods) {
	if (key == GLFW_KEY_UNKNOWN)
		return;

	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	[[maybe_unused]] const float delta_time = strct_ptr->delta_time;
	camera_t &camera = strct_ptr->camera;
	player_t &player = strct_ptr->player;

	if (action == GLFW_PRESS) {
		strct_ptr->key_holded[key] = true;

		switch(key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			case GLFW_KEY_G:
				camera.switch_following_mode();
				break;
			case GLFW_KEY_F:
				player.switch_fly_mode();
				break;
			case GLFW_KEY_R:
				player.set_position(player.debug_position);
				break;
			default:
				break;
		}
	}

	if (action == GLFW_RELEASE) {
		strct_ptr->key_holded[key] = false;
	}
}
