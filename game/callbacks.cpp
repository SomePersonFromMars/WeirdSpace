#include "callbacks.hpp"

#include "utils/useful.hpp"

callbacks_strct_t::callbacks_strct_t(
		GLFWwindow *window,
		GLint &window_width,
		GLint &window_height,
		double &delta_time,
		camera_t &camera
	)
	:window_width{window_width}
	,window_height{window_height}
	,delta_time{delta_time}
	,camera{camera}
{
	glfwSetWindowUserPointer(
			window,
			reinterpret_cast<void*>(this)
		);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);
}

void callbacks_strct_t::handle_input() {
	camera.enable_moving_acceleration(key_holded[GLFW_KEY_K]);
	camera.enable_rotation_acceleration(key_holded[GLFW_KEY_LEFT_SHIFT]);

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
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	strct_ptr->window_width = width;
	strct_ptr->window_height = height;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window,
		int key, int scancode, int action, int mods) {

	callbacks_strct_t * const strct_ptr
		= callbacks_strct_t::get_strct(window);

	const float delta_time = strct_ptr->delta_time;
	camera_t &camera = strct_ptr->camera;

	if (action == GLFW_PRESS) {
		strct_ptr->key_holded[key] = true;

		switch(key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			default:
				break;
		}
	}

	if (action == GLFW_RELEASE) {
		strct_ptr->key_holded[key] = false;
	}
}
