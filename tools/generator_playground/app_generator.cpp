#include "app.hpp"

#include <useful.hpp>

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

void app_t::init_map_generator() {
	callbacks_strct.map_generator = &map_generator;

	// GL_GET_ERROR

	map_storage.load_settings();
	map_storage.init_gl();
	map_storage.reallocate_gpu_and_cpu_memory();

	map_generator.load_settings();
	map_generator.init_gl();

	line.init();

	map_generator.new_seed();
	map_generator.generate_map();

	// map_storage.load_from_gpu_to_cpu_memory();
	// PRINT_D((int)map_storage.get_component_value(0, 0, 0));
}


void app_t::in_loop_draw_map() {
	// Drawing the map storage
	map_storage.draw(MVPb);

	// Drawing the line (player)
	if (global_settings.draw_player and
			not global_settings.generate_with_gpu and
			map_generator.are_tour_path_points_generated()) {
		mat4 MVP(1);
		MVP = scale(MVP, vec3(camera_zoom));
		MVP
		= scale(MVP, vec3(1, float(window_width)/float(window_height), 1));
		MVP = translate(MVP, -camera_pos *
				vec3(1, float(window_width)/float(window_height), 1)
			);
		auto [world_pos, gradient]
			= map_generator.get_tour_path_points(line_off);
		const float gradient_len
			= static_cast<float>(std::sqrt(len_sq(gradient)));
		const float gradient_sin
			= -static_cast<float>(gradient.x) / gradient_len;
		const float gradient_cos
			= static_cast<float>(gradient.y) / gradient_len;
		const mat4 rotate_mat {
			gradient_cos, -gradient_sin, 0, 0,
			gradient_sin, gradient_cos, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
		world_pos.x /= map_generator.get_space_max().x*3.0;
		world_pos.y /= map_generator.get_space_max().y;
		world_pos.y = 1.0 - world_pos.y;
		world_pos *= 2.0;
		world_pos.x -= 1.0;
		world_pos.y -= 1.0;
		world_pos.y
			= world_pos.y * map_generator.get_ratio_hw();
		MVP = translate(MVP, vec3(static_cast<vec2>(world_pos), 0.0));
		MVP *= rotate_mat;
		MVP = scale(MVP, vec3(vec2(4, 1) / 400.0f, 1.0f));
		line.draw(MVP);
	}
}

void app_t::deinit_map_generator() {
	map_generator.deinit_gl();
	map_storage.deinit_gl();
}

// Actions
void app_t::soft_reload_procedure() {
	map_storage.load_settings();
	map_storage.reallocate_gpu_and_cpu_memory();
	map_generator.load_settings();
	map_generator.generate_map();
};

void app_t::reload_procedure() {
	map_storage.load_settings();
	map_storage.reallocate_gpu_and_cpu_memory();
	map_generator.load_settings();
	map_generator.new_seed();
	map_generator.generate_map();
};
