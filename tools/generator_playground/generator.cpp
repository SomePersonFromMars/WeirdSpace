#include "generator.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "useful.hpp"
#include "settings.hpp"

#include "noise.hpp"

generator_t::generator_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
{
	noise.border_end = double(width)*noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= double(CHUNK_DIM)*noise_pos_mult*0.1;

	new_seed();
}

void generator_t::new_seed() {
	seed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 100;

	noise.reseed(seed);
}

u8vec3 generator_t::get(ivec2 ipos) {
	const int x = ipos.x;
	const int y = ipos.y;

	// int octaves = (1.0 - double(y)/double(height)) * 8.0;

	double val;
	val = noise.octave2D_01(
		double(x) * noise_pos_mult,
		double(y) * noise_pos_mult,
		8,
		// octaves,
		0.5
	)*1.0;
	// val = 1.0 - val;

	// val *= val;
	const double t = double(y)/double(height)*4.0/3.0;
	val = lerp(val*0.5+0.5, val*val, t*t*t*t);

	uint8_t val_u8 = static_cast<uint8_t>(val * 255.0);
	return u8vec3(val_u8);

	vec3 val_v3 = lerp<vec3, float>(
		// color_hex_to_vec3(0x826348),
		color_hex_to_vec3(0x826948),

		color_hex_to_vec3(0xf9d186),
		val);

	// vec3 val_v3(1);

	// return val_u8v3;
	return u8vec3(
		static_cast<uint8_t>(val_v3.r * 255.0),
		static_cast<uint8_t>(val_v3.g * 255.0),
		static_cast<uint8_t>(val_v3.b * 255.0)
	);
}


void generator_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {

#define SCENE 1
#if SCENE == 1
	for (int x = 0; x < bitmap.WIDTH; x += resolution_div)
		for (int y = 0; y < bitmap.HEIGHT; y += resolution_div) {
			const u8vec3 val = get({x, y});

			for (int dx = 0; dx < resolution_div; ++dx)
				for (int dy = 0; dy < resolution_div; ++dy) {
					if (x+dx >= width) continue;
					if (y+dy >= height) continue;

					bitmap.get(y+dy, x+dx, 0) = val.r;
					bitmap.get(y+dy, x+dx, 1) = val.g;
					bitmap.get(y+dy, x+dx, 2) = val.b;
				}
		}


	// World border
	for (int x = 0; x < bitmap.WIDTH; ++x) {
		bitmap.get(0, x, 2) = 0xff;
		bitmap.get(bitmap.HEIGHT-1, x, 2) = 0xff;
	}
	for (int y = 0; y < bitmap.HEIGHT; ++y) {
		bitmap.get(y, 0, 2) = 0xff;
		bitmap.get(y, bitmap.WIDTH-1, 2) = 0xff;
	}

	// Chunks visualization
	for (int x = CHUNK_DIM; x < bitmap.WIDTH; x += CHUNK_DIM) {

		for (int y = bitmap.HEIGHT/8*7; y < bitmap.HEIGHT; ++y) {
			bitmap.get(y, x, 0) = 0xff;
		}
	}

#elif SCENE == 2

#endif
#undef SCENE
}
