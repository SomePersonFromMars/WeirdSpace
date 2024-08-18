// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef GLOBAL_SETTINGS_GUI_HPP
#define GLOBAL_SETTINGS_GUI_HPP

#include <GLFW/glfw3.h>

class global_settings_gui {
public:
	static void draw_imgui_widgets();

    static void draw_global_settings_controls();
    static void draw_general_settings();
    static void draw_rivers_and_climate_settings();
};

#endif
