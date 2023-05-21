#include "app.hpp"

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

void app_t::init_generator() {
	callbacks_strct.generator = generator;
	bitmap_A.init();
	line.init();

	soft_reload_procedure();
}


void app_t::loop_generator() {
	// Drawing the bitmap
	{
		// bitmap_A.set(mp.y, mp.x + bitmap_A.WIDTH/3, 0xff0000);
		// bitmap_A.set(85, mp.x + bitmap_A.WIDTH/3, 0xff0000);
		// bitmap_A.set(85, 135 + bitmap_A.WIDTH/3, 0xffffff);
		bitmap_A.load_to_texture();
		bitmap_A.draw(MVPb);
	}

	// // Drawing the line (player)
	// {
	// 	mat4 MVP(1);
	// 	MVP = scale(MVP, vec3(camera_zoom));
	// 	MVP
	// 	= scale(MVP, vec3(1, float(window_width)/float(window_height), 1));
	// 	MVP = translate(MVP, -camera_pos *
	// 			vec3(1, float(window_width)/float(window_height), 1)
	// 		);
	// 	auto [world_pos, gradient]
	// 		= generator_C.get_tour_path_points(line_off);
	// 	const float gradient_len
	// 		= static_cast<float>(std::sqrt(len_sq(gradient)));
	// 	const float gradient_sin
	// 		= -static_cast<float>(gradient.x) / gradient_len;
	// 	const float gradient_cos
	// 		= static_cast<float>(gradient.y) / gradient_len;
	// 	const mat4 rotate_mat {
	// 		gradient_cos, -gradient_sin, 0, 0,
	// 		gradient_sin, gradient_cos, 0, 0,
	// 		0, 0, 1, 0,
	// 		0, 0, 0, 1,
	// 	};
	// 	world_pos.x /= generator_C.space_max.x;
	// 	world_pos.y /= generator_C.space_max.y;
	// 	world_pos.y = 1.0 - world_pos.y;
	// 	world_pos *= 2.0;
	// 	world_pos.x -= 1.0;
	// 	world_pos.y -= 1.0;
	// 	world_pos.y
	// 		= world_pos.y * generator->ratio_hw;
	// 	MVP = translate(MVP, vec3(static_cast<vec2>(world_pos), 0.0));
	// 	MVP *= rotate_mat;
	// 	MVP = scale(MVP, vec3(vec2(4, 1) / 400.0f, 1.0f));
	// 	line.draw(MVP);
	// }
}


// Actions
void app_t::soft_reload_procedure() {
	generator->generate_bitmap(bitmap_A);
	bitmap_A.load_to_texture();
};

void app_t::reload_procedure() {
	generator->load_settings();
	generator->new_seed();

	soft_reload_procedure();
};
