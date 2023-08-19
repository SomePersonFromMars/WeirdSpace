#include "callbacks.hpp"

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
	camera.enable_moving_acceleration(key_holded[GLFW_KEY_K]);
	camera.enable_rotation_acceleration(key_holded[GLFW_KEY_LEFT_SHIFT]);
	player.enable_moving_acceleration(key_holded[GLFW_KEY_Z]);

	if (key_holded[GLFW_KEY_UP])
		camera.rotate_up(delta_time);

	if (key_holded[GLFW_KEY_DOWN])
		camera.rotate_down(delta_time);

	if (key_holded[GLFW_KEY_RIGHT])
		camera.rotate_right(delta_time);

	if (key_holded[GLFW_KEY_LEFT])
		camera.rotate_left(delta_time);


	if (key_holded[GLFW_KEY_W])
		camera.move_forward(delta_time);

	if (key_holded[GLFW_KEY_S])
		camera.move_backward(delta_time);

	if (key_holded[GLFW_KEY_D])
		camera.move_right(delta_time);

	if (key_holded[GLFW_KEY_A])
		camera.move_left(delta_time);


	if (key_holded[GLFW_KEY_KP_8])
		player.move_up(delta_time);

	if (key_holded[GLFW_KEY_KP_2])
		player.move_down(delta_time);

	if (key_holded[GLFW_KEY_KP_6])
		player.move_right(delta_time);

	if (key_holded[GLFW_KEY_KP_4])
		player.move_left(delta_time);

	if (key_holded[GLFW_KEY_KP_0])
		player.jump(delta_time);


	[[maybe_unused]] const float off = 3.0 * delta_time;
	if (key_holded[GLFW_KEY_KP_7]) {
		// player.move_by({off, off});
		player.move_up(delta_time);
		player.move_left(delta_time);
	}

	if (key_holded[GLFW_KEY_KP_1]) {
		// player.move_by({off, -off});
		player.move_down(delta_time);
		player.move_left(delta_time);
	}

	if (key_holded[GLFW_KEY_KP_9]) {
		// player.move_by({-off, off});
		player.move_up(delta_time);
		player.move_right(delta_time);
	}

	if (key_holded[GLFW_KEY_KP_3]) {
		// player.move_by({-off, -off});
		player.move_down(delta_time);
		player.move_right(delta_time);
	}
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
			case GLFW_KEY_F:
				camera.switch_following_mode();
				break;
			case GLFW_KEY_G:
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
