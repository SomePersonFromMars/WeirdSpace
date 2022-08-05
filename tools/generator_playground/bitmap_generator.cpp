#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "useful.hpp"
#include "settings.hpp"

#include "bitmap.hpp"
#include "noise.hpp"

#define PATTERN 2

void generate_bitmap(bitmap_t &bitmap) {
	for (int x = 0; x < bitmap.WIDTH; ++x)
		for (int y = 0; y < bitmap.HEIGHT; ++y) {
			bitmap.content[y][x][0] = 0;
			bitmap.content[y][x][1] = 0;
			bitmap.content[y][x][2] = 0;
		}

	auto seed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1000;

#if PATTERN == 1

	std::srand(seed);
	constexpr int PIX_DIM = 3;

	glm::ivec2 pos(bitmap.WIDTH/2/PIX_DIM, bitmap.HEIGHT/2/PIX_DIM);
	for (int step = 0; step < 500; ++step) {

		for (int x = 0; x < PIX_DIM; ++x)
			for (int y = 0; y < PIX_DIM; ++y) {
				glm::ivec2 tpos = pos;
				tpos.x *= PIX_DIM;
				tpos.y *= PIX_DIM;

				tpos.x += x;
				tpos.y += y;

				tpos.x = std::min(tpos.x, bitmap.WIDTH-1);
				tpos.y = std::min(tpos.y, bitmap.HEIGHT-1);

				uint32_t color = rand() & 0xff'ff'ff;
				// uint32_t color = 0xffffff;
				bitmap.content[tpos.y][tpos.x][0] = (color & 0x0000ff) >> 0;
				bitmap.content[tpos.y][tpos.x][1] = (color & 0x00ff00) >> 8;
				bitmap.content[tpos.y][tpos.x][2] = (color & 0xff0000) >> 16;
			}

		pos.x += rand()%3-1;
		pos.y += rand()%3-1;
		pos.x = glm::clamp(pos.x, 0, bitmap.WIDTH/PIX_DIM);
		pos.y = glm::clamp(pos.y, 0, bitmap.HEIGHT/PIX_DIM);
	}

#elif PATTERN == 2

	// const double noise_pos_mult = 0.04;
	const double noise_pos_mult = 1.0/512.0*8.0;
	const int chunk_dim = 512;

	// siv::PerlinNoise noise;
	cyclic_noise_t noise;
	noise.reseed(seed);
	noise.border_end = double(bitmap.WIDTH)*noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= double(chunk_dim)*noise_pos_mult*0.1;

	for (int x = 0; x < bitmap.WIDTH; ++x)
		for (int y = 0; y < bitmap.HEIGHT; ++y) {

			// double val = perlin(
			// 		double(x)/double(bitmap.WIDTH-1 ) * 20.0,
			// 		double(y)/double(bitmap.HEIGHT-1) * 20.0
			// 	);

			double val;
			val = noise.octave2D_01(
			// val = noise.normalizedOctave2D_01(
					double(x) * noise_pos_mult,
					double(y) * noise_pos_mult,
					8
				)*1.0;

			uint8_t val_uint8 = static_cast<uint8_t>(val * 255.0);

			bitmap.content[y][x][0] = val_uint8;
			bitmap.content[y][x][1] = val_uint8;
			bitmap.content[y][x][2] = val_uint8;

			// PRINT_F(val);
		}

#endif

	for (int x = 0; x < bitmap.WIDTH; ++x) {
		bitmap.content[0][x][2] = 0xff;
		bitmap.content[bitmap.HEIGHT-1][x][2] = 0xff;
	}
	// for (int y = bitmap.HEIGHT/8*7; y < bitmap.HEIGHT; ++y) {
	for (int y = 0; y < bitmap.HEIGHT; ++y) {
		bitmap.content[y][0][2] = 0xff;
		bitmap.content[y][bitmap.WIDTH-1][2] = 0xff;
	}
}
