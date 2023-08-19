#pragma once
#ifndef APP_HPP
#define APP_HPP

#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "map_generator/map_storage.hpp"
#include "map_generator/map_generator.hpp"
#include "shader_A.hpp"
#include "shader_world.hpp"
#include "world_buffer.hpp"
#include "world_generator.hpp"
#include "player.hpp"
#include "callbacks.hpp"

struct app_t {
	app_t();

	void init();
	void loop();
	void deinit();

private:
	// Init subfunctions
	void init_opengl_etc();
	void init_imgui();
	void init_camera();
	void init_map_related();
	void init_world_blocks();
	void init_player();
	void init_callbacks();

	// Deinit subfunctions
	void deinit_opengl_etc();
	void deinit_imgui();
	void deinit_map_related();
	void deinit_world_blocks();
	void deinit_player();

	// Loop subfuncitons
	void in_loop_update_imgui();

	// Window
	GLFWwindow* window;
	GLint window_width = 1920;
	GLint window_height = 1080;

	// Settings
	const glm::vec3 background_color;

	// Time related
	double delta_time = 0.0;
	std::chrono::time_point<std::chrono::high_resolution_clock>
		timer, now;
	long int elapsed;

	// Camera
	camera_t camera;

	// World map
	map_storage_t map_storage;
	map_generator_t map_generator;

	// 3D world objects
	shader_A_t shader_A;
	shader_world_t shader_world;
	world_buffer_t world_buffer;
	world_generator_t world_generator;
	player_t player;

	// Callbacks
	callbacks_strct_t callbacks_strct;
};

#endif
