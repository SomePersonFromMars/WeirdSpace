#include "generator.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <vector>
#include <queue>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "useful.hpp"
#include "settings.hpp"

#include "noise.hpp"

generator_A_t::generator_A_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
{
	noise.border_end = double(width)*noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= double(CHUNK_DIM)*noise_pos_mult*0.1;

	new_seed();
}

void generator_A_t::new_seed() {
	cyclic_noise_t::seed_type seed;
	seed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 100;

	noise.reseed(seed);
}

u8vec3 generator_A_t::get(ivec2 ipos) {
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


void generator_A_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {

	for (int x = 0; x < bitmap.WIDTH; x += resolution_div)
		for (int y = 0; y < bitmap.HEIGHT; y += resolution_div) {
			const u8vec3 val = get({x, y});

			for (int dx = 0; dx < resolution_div; ++dx)
				for (int dy = 0; dy < resolution_div; ++dy) {
					if (x+dx >= width) continue;
					if (y+dy >= height) continue;

					// bitmap.get(y+dy, x+dx, 0) = val.r;
					// bitmap.get(y+dy, x+dx, 1) = val.g;
					// bitmap.get(y+dy, x+dx, 2) = val.b;
					bitmap.set(y+dy, x+dx, val);
				}
		}

	// // World border
	// for (int x = 0; x < bitmap.WIDTH; ++x) {
	// 	bitmap.get(0, x, 0) = 0;
	// 	bitmap.get(0, x, 1) = 0;
	// 	bitmap.get(0, x, 2) = 0xff;

	// 	bitmap.get(bitmap.HEIGHT-1, x, 0) = 0;
	// 	bitmap.get(bitmap.HEIGHT-1, x, 1) = 0;
	// 	bitmap.get(bitmap.HEIGHT-1, x, 2) = 0xff;
	// }
	// for (int y = 0; y < bitmap.HEIGHT; ++y) {
	// 	bitmap.get(y, 0, 0) = 0;
	// 	bitmap.get(y, 0, 1) = 0;
	// 	bitmap.get(y, 0, 2) = 0xff;

	// 	bitmap.get(y, bitmap.WIDTH-1, 0) = 0;
	// 	bitmap.get(y, bitmap.WIDTH-1, 1) = 0;
	// 	bitmap.get(y, bitmap.WIDTH-1, 2) = 0xff;
	// }

	// // Chunks visualization
	// for (int x = CHUNK_DIM; x < bitmap.WIDTH; x += CHUNK_DIM) {

	// 	for (int y = bitmap.HEIGHT/8*7; y < bitmap.HEIGHT; ++y) {
	// 		bitmap.get(y, x, 0) = 0xff;
	// 	}
	// }
}

generator_B_t::generator_B_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
	,ratio_wh{float(width)/float(height)}
	,ratio_hw{float(height)/float(width)}
{
}

void generator_B_t::new_seed() {
}

void generator_B_t::draw_edge(bitmap_t &bitmap, vec2 beg01, vec2 end01,
		uint32_t color) {
	vec2 beg, end;

	beg.x = beg01.x*float(width-1);
	beg.x /= float(width) / float(height);
	beg.y = beg01.y*float(height-1);

	end.x = end01.x*float(width-1);
	end.x /= float(width) / float(height);
	end.y = end01.y*float(height-1);

	// beg -= vec2(float(width), 0);
	// end -= vec2(float(width), 0);

	vec2 off = end - beg;
	const int iterations_cnt = std::max(
			std::abs(int(off.x)), std::abs(int(off.y)));
	if (std::abs(off.x) > std::abs(off.y)) {
		off /= std::abs(off.x);
	} else {
		off /= std::abs(off.y);
	}

	vec2 pos = beg;
	for (int i = 0; i < iterations_cnt; ++i, pos += off) {
		if (pos.x < 0 || pos.x >= float(width)
				|| pos.y < 0 || pos.y >= float(height))
			continue;

		// bitmap.get(int(pos.y), int(pos.x), 0) = 0xff;
		// bitmap.get(int(pos.y), int(pos.x), 1) = 0xff;
		// bitmap.get(int(pos.y), int(pos.x), 2) = 0xff;

		// bitmap.get(int(pos.y), int(pos.x), 0) = (color & 0x0000ff) >> 0;
		// bitmap.get(int(pos.y), int(pos.x), 1) = (color & 0x00ff00) >> 8;
		// bitmap.get(int(pos.y), int(pos.x), 2) = (color & 0xff0000) >> 16;

		bitmap.set(pos.y, pos.x, color);
	}
}

void generator_B_t::draw_point(bitmap_t &bitmap, glm::vec2 pos, float dim,
		uint32_t color) {

	ivec2 beg, end;
	beg.x = (pos.x-dim/2.0f)*float(width-1)/ratio_wh;
	end.x = (pos.x+dim/2.0f)*float(width-1)/ratio_wh;
	beg.y = (pos.y-dim/2.0f)*float(height-1);
	end.y = (pos.y+dim/2.0f)*float(height-1);

	for (int x = beg.x; x <= end.x; ++x) {
		if (x < 0 || x >= width) continue;

		for (int y = beg.y; y <= end.y; ++y) {
			if (y < 0 || y >= height) continue;

			// bitmap.get(y, x, 0) = 0xff;
			// bitmap.get(y, x, 1) = 0;
			// bitmap.get(y, x, 2) = 0;

			bitmap.set(y, x, color);
		}
	}
}

void generator_B_t::fill(bitmap_t &bitmap, glm::vec2 origin,
		uint32_t edge_color, uint32_t fill_color) {
	ivec2 first_pixel;
	first_pixel.x = origin.x * float(width-1) / ratio_wh;
	first_pixel.y = origin.y * float(height-1);
	if (first_pixel.x < 0 || first_pixel.x >= width)
		return;
	if (first_pixel.y < 0 || first_pixel.y >= height)
		return;

	static const ivec2 dir[] {
		ivec2(-1, 0),
		ivec2(1, 0),
		ivec2(0, -1),
		ivec2(0, 1)
	};

	std::queue<ivec2> next_pixel;
	next_pixel.push(first_pixel);
	bitmap.set(first_pixel.y, first_pixel.x, fill_color);

	while (!next_pixel.empty()) {
		ivec2 pixel = next_pixel.front();
		next_pixel.pop();

		for (int i = 0; i < 4; ++i) {
			const ivec2 d = dir[i];
			const ivec2 p = pixel+d;
			if (p.x < 0 || p.x >= width)
				continue;
			if (p.y < 0 || p.y >= height)
				continue;

			const uint32_t cur_color = bitmap.get(p.y, p.x);
			if (cur_color == fill_color || cur_color == edge_color)
				continue;

			next_pixel.push(p);
			bitmap.set(p.y, p.x, fill_color);
		}
	}
}

void generator_B_t::draw_hexagon(bitmap_t &bitmap, glm::ivec2 grid_pos,
		uint32_t edge_color, uint32_t fill_color) {

	grid_pos.x = (grid_pos.y&1) + grid_pos.x*3;

	const vec2 origin =
		vec2(float(grid_pos.x)*tri_edge + (grid_pos.y%2? tri_edge/2.0f : 0.0f),
		float(grid_pos.y)*tri_h*tri_edge) + vec2(tri_edge, tri_edge*tri_h);

	// Draw outline
	for (int k = 0; k < 6; ++k) {
		const int l = (k+1)%6;

		const vec2 A = vec2(hex_points[2*k],
				hex_points[2*k+1])*tri_edge+origin;
		const vec2 B = vec2(hex_points[2*l],
				hex_points[2*l+1])*tri_edge+origin;

		draw_edge(bitmap,
			A,
			B,
			edge_color
			);

		draw_point(bitmap, A, 2.0f/float(height-1), edge_color);
		draw_point(bitmap, B, 2.0f/float(height-1), edge_color);
	}

	// Fill the hexagon
	fill(bitmap, origin, edge_color, fill_color);

	// draw_point(bitmap,
	// 	origin+vec2(0, 0), 0.01, 0x0000ff);
}

void generator_B_t::generate_bitmap(bitmap_t &bitmap) {
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			bitmap.set(y, x, 0);
		}
	}

	// for (int i = 0; i < int(floor(3.0f/tri_edge)); ++i) {
	// 	for (int j = 0; j < int(floor(3.0f/tri_edge*ratio_wh)); ++j) {
	// 		draw_hexagon(bitmap, ivec2(j, i), 0xffffff, 0x000000);
	// 	}
	// }

	generate_grid({
			int(floor(1.0f/tri_edge*ratio_wh/3.0f)) - 0,
			int(floor(1.0f/tri_edge/tri_h/1.0f))    - 1,
			});
	for (int y = 0; y < plates.size(); ++y) {
		for (int x = 0; x < plates[y].size(); ++x) {
			draw_hexagon(bitmap, ivec2(x, y),
					(plates[y][x]+1)&0xffffff, plates[y][x]);

			// draw_hexagon(bitmap, ivec2(x, y), 0, plates[y][x]);
			// draw_hexagon(bitmap, ivec2(x, y), 0x123456, 0x111111);
		}
	}

	// const float triangle_width = 0.05f;
	// const float triangle_height = 0.866025404f * triangle_width;
	// for (int i = 0; i < int(ceil(1.0f/triangle_height)); ++i) {
	// 	for (int j = 0; j < int(ceil(1.0f/triangle_width*ratio_wh)); ++j) {
	// 		const vec2 origin
	// 			(float(j)*triangle_width + (i%2? triangle_width/2.0f : 0.0f),
	// 			float(i)*triangle_height);
	// 		const vec2 A(origin);
	// 		const vec2 B = origin + vec2(triangle_width, 0);
	// 		const vec2 C = origin + vec2(triangle_width/2.0f, triangle_height);
	// 		draw_edge(bitmap, A, B, 0xffffff);
	// 		draw_edge(bitmap, A, C, 0xffffff);
	// 		draw_edge(bitmap, B, C, 0xffffff);
	// 		if (j % 3 == (i & 1)) {
	// 			// draw_point(bitmap,
	// 			// 		origin+vec2(0, 0), 0.01);
	// 			// draw_hexagon(bitmap, ivec2(j, i), 0xffffff, 0xff2222);
	// 		}
	// 	}
	// }

	// const ivec2 v(2, 3);
	// draw_hexagon(bitmap, v, 0xdddddd, 0x777777);
	// // draw_hexagon(bitmap, v, 0xdddddd, hsv_to_rgb(1.0f, 0.6f, 0.8f));
	// for (int i = 0; i < 6; ++i) {
	// 	// draw_hexagon(bitmap, get_hex_neighbor(v, i), 0xdddddd, 0x555555);
	// 	draw_hexagon(bitmap, get_hex_neighbor(v, i), 0xdddddd,
	// 			hsv_to_rgb(float(i)/6.0f, 0.6f, 0.8f));
	// }

	// // World border
	// for (int x = 0; x < bitmap.WIDTH; ++x) {
	// 	bitmap.get(0, x, 0) = 0;
	// 	bitmap.get(0, x, 1) = 0;
	// 	bitmap.get(0, x, 2) = 0xff;

	// 	bitmap.get(bitmap.HEIGHT-1, x, 0) = 0;
	// 	bitmap.get(bitmap.HEIGHT-1, x, 1) = 0;
	// 	bitmap.get(bitmap.HEIGHT-1, x, 2) = 0xff;
	// }
	// for (int y = 0; y < bitmap.HEIGHT; ++y) {
	// 	bitmap.get(y, 0, 0) = 0;
	// 	bitmap.get(y, 0, 1) = 0;
	// 	bitmap.get(y, 0, 2) = 0xff;

	// 	bitmap.get(y, bitmap.WIDTH-1, 0) = 0;
	// 	bitmap.get(y, bitmap.WIDTH-1, 1) = 0;
	// 	bitmap.get(y, bitmap.WIDTH-1, 2) = 0xff;
	// }
}

glm::ivec2 generator_B_t::get_hex_neighbor(glm::ivec2 v, int id) {
	static const ivec2 dir[] {
		{0, 1}, // #
		{0, 2},
		{1, 1}, // #
		{1, -1}, // #
		{0, -2},
		{0, -1}, // #
	};
	return v + dir[id%6] + ( id%3!=1 && (v.y&1)==0
			? ivec2(-1, 0) : ivec2(0, 0));
}

void generator_B_t::generate_grid(glm::ivec2 size) {
	const std::mt19937::result_type seed
		= std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1;
	std::mt19937 gen(seed);
	std::uniform_int_distribution<> distrib_x(0, size.x-1);
	std::uniform_int_distribution<> distrib_y(0, size.y-1);
	std::uniform_int_distribution<> distrib_land(1, 100);

	plates.assign(size.y, std::vector<uint32_t>(size.x, 0));

	const int tectonic_plates_cnt = 80;
	std::queue<ivec2> next_v;
	std::set<ivec2, vec2_cmp_t<int>> points;
	for (int i = 0; i < tectonic_plates_cnt; ++i) {
		ivec2 p;
		while (
			points.find(
				p = ivec2(distrib_x(gen), distrib_y(gen)))
			!= points.end()
			)
			;
		points.insert(p);

		next_v.push(p);
		plates[p.y][p.x] = hsv_to_rgb(
				float(i)/float(tectonic_plates_cnt), 0.9f, 0.8f);

		if (distrib_land(gen) <= 40)
			plates[p.y][p.x] = 0x3a9648;
		else
			plates[p.y][p.x] = 0x77c4dd;
	}

	// static const std::vector<ivec2> points {
	// 	ivec2
	// 	// (0, 0),
	// 	// {0, 1},
	// 	// {0, 2},
	// 	// {0, 3},
	// 	(1, 3),
	// 	{3, 4},
	// 	{4, 5},
	// 	{6, 6},
	// 	{7, 7},
	// 	{9, 8},
	// };
	// for (int i = 0; i < points.size(); ++i) {
	// 	const ivec2 &p = points[i];
	// 	plates[p.y][p.x] = hsv_to_rgb(
	// 			float(i)/float(points.size()), 0.6f, 1.0f);
	// 	next_v.push(p);
	// }

	while (!next_v.empty()) {
		const ivec2 v = next_v.front();
		next_v.pop();

		for (int i = 0; i < 6; ++i) {
			const ivec2 w = get_hex_neighbor(v, i);
			if (w.x < 0 || w.x >= size.x)
				continue;
			if (w.y < 0 || w.y >= size.y)
				continue;
			if (plates[w.y][w.x] != 0)
				continue;

			plates[w.y][w.x] = plates[v.y][v.x];
			next_v.push(w);
			// plates[w.y][w.x] = 0x555555;
		}
	}

	// for (int y = 0; y < plates.size(); ++y) {
	// 	for (int x = 0; x < plates[y].size(); ++x) {
	// 		plates[y][x] = 0x555555;
	// 	}
	// }
}
