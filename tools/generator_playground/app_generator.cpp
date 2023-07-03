#include "app.hpp"

#include "useful.hpp"

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

void app_t::init_generator() {
	callbacks_strct.generator_C = &generator_C;

	// GL_GET_ERROR
	PRINT_U(GL_NO_ERROR);
	PRINT_U(GL_INVALID_ENUM);
	PRINT_U(GL_INVALID_VALUE);
	PRINT_U(GL_INVALID_OPERATION);
	PRINT_U(GL_INVALID_FRAMEBUFFER_OPERATION);
	PRINT_U(GL_OUT_OF_MEMORY);
	PRINT_U(GL_STACK_UNDERFLOW);
	PRINT_U(GL_STACK_OVERFLOW);

	bitmap_A.init();
	generator_C.init();
	line.init();
	soft_reload_procedure();
}


void app_t::loop_generator() {
	// Drawing the bitmap
	{
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
	// 		= world_pos.y * generator_C.ratio_hw;
	// 	MVP = translate(MVP, vec3(static_cast<vec2>(world_pos), 0.0));
	// 	MVP *= rotate_mat;
	// 	MVP = scale(MVP, vec3(vec2(4, 1) / 400.0f, 1.0f));
	// 	line.draw(MVP);
	// }
}


// Actions
void app_t::soft_reload_procedure() {
	bitmap_A.load_settings();
	generator_C.load_settings();

	generator_C.generate_bitmap();
};

void app_t::reload_procedure() {
	generator_C.new_seed();

	soft_reload_procedure();
};
