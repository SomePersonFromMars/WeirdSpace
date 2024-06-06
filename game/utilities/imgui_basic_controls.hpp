// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef IMGUI_BASIC_CONTROLS_HPP
#define IMGUI_BASIC_CONTROLS_HPP

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class imgui_basic_controls {
public:
	static void init_imgui(GLFWwindow *glfw_window);
	static void begin_drawing();
	static void end_drawing();
	static void deinit_imgui();

	static void draw_demo_windows();
};

#endif
