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
#include <delaunator.hpp>

generator_t::generator_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
	,ratio_wh{float(width)/float(height)}
	,ratio_hw{float(height)/float(width)}
{
	// new_seed();
}

generator_A_t::generator_A_t()
	:generator_t()
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
	:generator_t()
{
	new_seed();
}

void generator_B_t::new_seed() {
	seed_voronoi = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1;

	cyclic_noise_t::seed_type seed_noise;
	seed_noise = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1;
	coast_noise.reseed(seed_noise);
}

void generator_t::draw_edge(bitmap_t &bitmap, vec2 beg01, vec2 end01,
		uint32_t color) {
	dvec2 beg, end;

	beg.x = beg01.x*double(width-1);
	beg.x /= double(width) / double(height);
	beg.y = beg01.y*double(height-1);

	end.x = end01.x*double(width-1);
	end.x /= double(width) / double(height);
	end.y = end01.y*double(height-1);

	dvec2 off = end - beg;
	const int iterations_cnt = std::max(
			std::abs(int(off.x)), std::abs(int(off.y)));
	if (std::abs(off.x) > std::abs(off.y)) {
		off /= std::abs(off.x);
	} else {
		off /= std::abs(off.y);
	}

	dvec2 pos = beg;
	for (int i = 0; i < iterations_cnt; ++i, pos += off) {
		if (pos.x < 0 || pos.x >= double(width)
				|| pos.y < 0 || pos.y >= double(height))
			continue;
		bitmap.set(pos.y, pos.x, color);
	}
}

void generator_t::draw_ray(bitmap_t &bitmap, glm::vec2 beg01, glm::vec2 mid01,
		uint32_t color) {

	bool on_left = beg01.x < 0.0f;
	bool on_right = beg01.x > space_max.x;
	bool on_bottom = beg01.y < 0.0f;
	bool on_top = beg01.y > space_max.y;
	if (on_left || on_right || on_bottom || on_top) {
		bool drawable = false;
		vec2 intersection;
		vec2 off01 = mid01 - beg01;
		if (!drawable && on_left && off01.x > 0) {
			intersection.x = 0.0f;
			intersection.y =
				beg01.y + off01.y * (intersection.x - beg01.x) / off01.x;
			drawable = in_between_inclusive(0.0f, space_max.y, intersection.y);
		}
		if (!drawable && on_right && off01.x < 0) {
			intersection.x = space_max.x;
			intersection.y =
				beg01.y + off01.y * (intersection.x - beg01.x) / off01.x;
			drawable = in_between_inclusive(0.0f, space_max.y, intersection.y);
		}
		if (!drawable && on_bottom && off01.y > 0) {
			intersection.y = 0.0f;
			intersection.x =
				beg01.x + off01.x * (intersection.y - beg01.y) / off01.y;
			drawable = in_between_inclusive(0.0f, space_max.x, intersection.x);
		}
		if (!drawable && on_top && off01.y < 0) {
			intersection.y = space_max.y;
			intersection.x =
				beg01.x + off01.x * (intersection.y - beg01.y) / off01.y;
			drawable = in_between_inclusive(0.0f, space_max.x, intersection.x);
		}

		if (drawable) {
			beg01 = intersection;
			mid01 = beg01 + off01;
		} else {
			return;
		}
	}

	vec2 beg, mid;

	beg.x = beg01.x*float(width-1);
	beg.x /= float(width) / float(height);
	beg.y = beg01.y*float(height-1);

	mid.x = mid01.x*float(width-1);
	mid.x /= float(width) / float(height);
	mid.y = mid01.y*float(height-1);

	vec2 off = mid - beg;
	if (std::abs(off.x) > std::abs(off.y)) {
		off /= std::abs(off.x);
	} else {
		off /= std::abs(off.y);
	}

	vec2 pos = beg;
	for (; ; pos += off) {
		if (pos.x < 0 || pos.x >= float(width)
				|| pos.y < 0 || pos.y >= float(height))
			break;
		bitmap.set(pos.y, pos.x, color);
	}
}

void generator_t::draw_point(bitmap_t &bitmap, glm::vec2 pos, float dim,
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

void generator_B_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {
	bitmap.clear();

	// Generate and draw the grid
	grids[0] = { ivec2(-1), 64,  0.40f };
	// grids[1] = { ivec2(-1), 512,  0.00f };
	// grids[2] = { ivec2(-1), 2048, 0.00f };
	// grids[3] = { ivec2(-1), 2048, 0.00f };
	// grids[4] = { ivec2(-1), 4096, 0.00f };

	for (auto &grid : grids) {
		grid.size.x = 512;
		grid.size.y = grid.size.x * ratio_hw;
		grid.generate_grid(seed_voronoi, coast_noise);
	}

	static constexpr uint32_t WATER_COLOR = 0x77c4dd;
	// static constexpr uint32_t COAST_COLOR = 0xfcf04b;
	static constexpr uint32_t LAND_COLOR = 0x3a9648;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const ivec2 v(
					x * grids[0].size.x / width,
					y * grids[0].size.y / height
					);
			const tile_t &tile = grids[0].grid[v.y][v.x];
			const uint32_t type = tile.type;

			// dvec3 pos;
			// pos.y = y / double(height-1);
			// pos.x = x / double(height-1);
			// pos *= 5.0;
			// const double noise_val =
			// 	coast_noise.octave2D_01(pos.x, pos.y, 8);

			// u8vec3 color;
			// color.r = noise_val * 255.0;
			// color.g = color.b = color.r;
			// bitmap.set(y, x, color);
			// continue;

			// const double tmp = grids[0].grid
			// 		[y * grids[0].size.y / height]
			// 		[x * grids[0].size.x / width].tmp;
			// bitmap.set(y, x, u8vec3(tmp*255.0, tmp*255.0, tmp*255.0));
			// continue;

			// uint32_t type = 0;
			// for (auto &grid : grids) {
			// 	type |= grid.grid
			// 			[y * grid.size.y / height]
			// 			[x * grid.size.x / width].type;
			// }

			if (type & tile_t::COAST_BIT) {
				const float depth = std::sqrt(
						float(len_sq(v - tile.coast_origin))) /
					float(tile_t::COAST_DEPTH+1);
				// const float depth = 1.0f -
				// 	float(tile.coast_dist) / float(tile_t::COAST_DEPTH);
				u8vec3 color(depth*255.0f);
				// const u8vec3 color(tile.tmp*255.0f);

				if (tile.perturbtion <= tile_t::COAST_DEPTH)
					bitmap.set(y, x, WATER_COLOR);
					// bitmap.set(y, x, COAST_COLOR);
					// ;
				else
					// ;
					// color.g = 0xffu/2u;
					bitmap.set(y, x, LAND_COLOR);
				// bitmap.set(y, x, color);
				// bitmap.set(y, x, COAST_COLOR);

				// const u8vec3 a = color_hex_to_u8vec3(LAND_COLOR);
				// const u8vec3 b = color_hex_to_u8vec3(WATER_COLOR);
				// u8vec3 color;
				// color.r = lerp(a.r, b.r, noise_val);
				// color.g = lerp(a.g, b.g, noise_val);
				// color.b = lerp(a.b, b.b, noise_val);
				// bitmap.set(y, x, color);

				// if (noise_val < 0.5)
				// 	bitmap.set(y, x, LAND_COLOR);
				// else
				// 	bitmap.set(y, x, WATER_COLOR);
			} else if (type & tile_t::LAND_BIT) {
				bitmap.set(y, x, LAND_COLOR);
			} else {
				bitmap.set(y, x, WATER_COLOR);
			}
		}
	}
}

void generator_B_t::fractal_grid_t::generate_grid(
		std::mt19937::result_type seed_voronoi, noise_t &noise) {
	std::mt19937 gen(seed_voronoi);
	std::uniform_int_distribution<> distrib_x(0, size.x-1);
	std::uniform_int_distribution<> distrib_y(0, size.y-1);
	std::uniform_real_distribution<float> distrib_land(0.0f, 1.0f);

	grid.assign(size.y, std::vector<tile_t>(size.x, {0}));

	std::queue<ivec2> next_v_A, next_v_B;
	std::set<ivec2, vec2_cmp_t<int>> points;
	for (int i = 0; i < voronoi_cnt; ++i) {
		ivec2 p;
		while (
			points.find(
				p = ivec2(distrib_x(gen), distrib_y(gen)))
			!= points.end()
			)
			;
		points.insert(p);

		next_v_A.push(p);
		// grid[p.y][p.x] = hsv_to_rgb(
		// 		float(i)/float(voronoi_cnt), 0.9f, 0.8f);

		if (distrib_land(gen) <= land_probability)
			grid[p.y][p.x].type = tile_t::LAND_BIT;
		else
			grid[p.y][p.x].type = tile_t::WATER_BIT;
	}

	static const std::array<ivec2, 4> dir {
		ivec2(-1, 0),
		ivec2(1, 0),
		ivec2(0, -1),
		ivec2(0, 1),
	};

	while (!next_v_A.empty()) {
		const ivec2 v = next_v_A.front();
		auto &grid_v = grid[v.y][v.x];
		next_v_A.pop();

		for (const ivec2 &d : dir) {
			const ivec2 w = v + d;
			if (w.x < 0 || w.x >= size.x)
				continue;
			if (w.y < 0 || w.y >= size.y)
				continue;
			auto &grid_w = grid[w.y][w.x];

			if (grid_v.type == tile_t::LAND_BIT &&
					grid_w.type == tile_t::WATER_BIT) {
				next_v_B.push(v);
				grid_v.type = tile_t::COAST_BIT;
				grid_v.coast_origin = v;

				dvec2 pos;
				pos.x = w.x / double(size.y-1);
				pos.y = w.y / double(size.y-1);
				pos *= 10.0;
				const double val = noise.octave2D_01(pos.x, pos.y, 8);
				// grid_w.tmp = val;
				grid_v.perturbtion = val * tile_t::COAST_DEPTH;
			}
			if (grid_w.type != 0)
				continue;

			if (grid_v.type != tile_t::COAST_BIT)
				grid_w.type = grid_v.type;
			else
				grid_w.type = tile_t::LAND_BIT;
			next_v_A.push(w);
		}
	}

	while (!next_v_B.empty()) {
		const ivec2 v = next_v_B.front();
		const auto &grid_v = grid[v.y][v.x];
		next_v_B.pop();

		for (const ivec2 &d : dir) {
			const ivec2 w = v + d;
			if (w.x < 0 || w.x >= size.x)
				continue;
			if (w.y < 0 || w.y >= size.y)
				continue;
			auto &grid_w = grid[w.y][w.x];
			// if (grid_w.type != tile_t::LAND_BIT)
			// 	continue;
			if (grid_w.type == tile_t::WATER_BIT)
				continue;

			const long long cur_dist_sq = len_sq(w - grid_w.coast_origin);
			const long long new_dist_sq = len_sq(w - grid_v.coast_origin);
			if (grid_w.type == tile_t::COAST_BIT) {
				if (cur_dist_sq <= new_dist_sq)
					continue;
			}

			grid_w.type = tile_t::COAST_BIT;
			// grid_w.coast_dist = grid_v.coast_dist + 1;
			grid_w.coast_origin = grid_v.coast_origin;
			grid_w.perturbtion = grid_v.perturbtion + 1;
			// if (grid_w.coast_dist < tile_t::COAST_DEPTH)
			// 	next_v_B.push(w);
			if (new_dist_sq < tile_t::COAST_DEPTH_SQ)
				next_v_B.push(w);
		}
	}
}

generator_C_t::generator_C_t()
	:generator_t()
{
	new_seed();
}

void generator_C_t::new_seed() {
	seed_voronoi = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1;
}

glm::vec2 generator_C_t::triangle_circumcenter(
		glm::vec2 A, glm::vec2 B, glm::vec2 C) const {

	const double A_len_sq = len_sq(A);
	const double B_len_sq = len_sq(B);
	const double C_len_sq = len_sq(C);

	const double denominator
		= A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y);

	vec2 O(0, 0);
	O.x += A_len_sq * (B.y - C.y);
	O.x += B_len_sq * (C.y - A.y);
	O.x += C_len_sq * (A.y - B.y);
	O.x /= 2.0;
	O.x /= denominator;

	O.y += A_len_sq * (B.x - C.x);
	O.y += B_len_sq * (C.x - A.x);
	O.y += C_len_sq * (A.x - B.x);
	O.y /= -2.0;
	O.y /= denominator;

	return O;
}

void generator_C_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {
	bitmap.clear();
	// draw_point(bitmap, vec2(0.1, 0.1), 0.01, 0x00ff00);

	// Buggy seeds meant for 5 points
	// seed_voronoi = 1662653759901; // Fixed
	// Lacks one polygon, Fixed:
	// seed_voronoi = 1662654644174;
	// seed_voronoi = 1662841001959;
	// Weird double edges, Fixed
	// seed_voronoi = 1662841190818;
	// seed_voronoi = 1662841295870; // Best
	// seed_voronoi = 1662841349208;
	// seed_voronoi = 1662841708166;
	// seed_voronoi = 1663425450766;
	// seed_voronoi = 1663425738310;
	// seed_voronoi = 1663425800255;
	// Buggy seeds meant for 120 points:
	// Very long rasterization:
	// seed_voronoi = 1663428536766;

	// Example seed:
	// seed_voronoi = 1663429230839;

	std::mt19937 gen(seed_voronoi);
	PRINT_LU(seed_voronoi);
	std::uniform_real_distribution<double> distrib_x(0,
			double(width-1)/double(width) * ratio_wh);
	std::uniform_real_distribution<double> distrib_y(0,
			double(height-1)/double(height));

	constexpr std::size_t voronoi_cnt = 120;
	// constexpr std::size_t voronoi_cnt = 5;
	std::vector<double> coords(voronoi_cnt*2);
	printf("\n");
	for (std::size_t i = 0; i < voronoi_cnt; ++i) {
		coords[2*i+0] = distrib_x(gen);
		coords[2*i+1] = distrib_y(gen);
		// printf("vec2(%f, %f),\n", coords[2*i+0], coords[2*i+1]);
	}

	delaunator::Delaunator d(coords);
	std::vector<std::vector<std::size_t>> triangles_of_voronoi(voronoi_cnt);
	std::vector<bool> voronoi_complete(voronoi_cnt, false);
	std::vector<dvec2> tri_centroid(d.triangles.size() / 3);
	std::vector<dvec2> tri_circumcenter(d.triangles.size() / 3);
	for (std::size_t i = 0; i < d.triangles.size() / 3; ++i) {
		// d.triangles[i]
		// d.triangles[i+1]
		// d.triangles[i+2]

		// triangles_of_voronoi[d.triangles[3*i]].push_back(i);
		// triangles_of_voronoi[d.triangles[3*i+1]].push_back(i);
		// triangles_of_voronoi[d.triangles[3*i+2]].push_back(i);

		dvec2 A, B, C;
		A.x = d.coords[2 * d.triangles[3*i]];
		A.y = d.coords[2 * d.triangles[3*i] + 1];
		B.x = d.coords[2 * d.triangles[3*i + 1]];
		B.y = d.coords[2 * d.triangles[3*i + 1] + 1];
		C.x = d.coords[2 * d.triangles[3*i + 2]];
		C.y = d.coords[2 * d.triangles[3*i + 2] + 1];

		tri_centroid[i].x = (A.x + B.x + C.x) / 3.0;
		tri_centroid[i].y = (A.y + B.y + C.y) / 3.0;
		tri_circumcenter[i] = triangle_circumcenter(A, B, C);

		// draw_edge(bitmap, A, B, 0x0000ff);
		// draw_edge(bitmap, A, C, 0x0000ff);
		// draw_edge(bitmap, B, C, 0x0000ff);

		// printf(
		// 		"Triangle points: [[%f, %f], [%f, %f], [%f, %f]]\n",
		// 		d.coords[2 * d.triangles[i]],        //tx0
		// 		d.coords[2 * d.triangles[i] + 1],    //ty0
		// 		d.coords[2 * d.triangles[i + 1]],    //tx1
		// 		d.coords[2 * d.triangles[i + 1] + 1],//ty1
		// 		d.coords[2 * d.triangles[i + 2]],    //tx2
		// 		d.coords[2 * d.triangles[i + 2] + 1] //ty2
		// 	  );
	}

	// for (std::size_t i = 0; i < voronoi_cnt; ++i)
	// 	draw_point(
	// 		bitmap, dvec2(coords[2*i], coords[2*i+1]), 0.005f, 0xff0000);

	for (std::size_t i = 0; i < d.triangles.size(); i += 3) {
		// draw_point(bitmap, tri_circumcenter[i/3], 0.005f, 0x00ff00);
		// PRINT_VEC2(tri_circumcenter[i/3]);
	}

	for (std::size_t i = 0; i < d.triangles.size(); ++i) {
		if (d.halfedges[i] == delaunator::INVALID_INDEX) {
			const dvec2 v1(
					coords[2 * d.triangles[i]],
					coords[2 * d.triangles[i] + 1]);
			const dvec2 v2(
					coords[2 * d.triangles[i-i%3+(i+1)%3]],
					coords[2 * d.triangles[i-i%3+(i+1)%3] + 1]);
			const dvec2 &P = v1;
			const dvec2 &Q = tri_circumcenter[i/3];
			const dvec2 v = v2 - v1;
			const double &A = v.x;
			const double &B = v.y;
			if (A == 0 && B == 0)
				continue;
			const double S = P.x - Q.x;
			const double T = Q.y - P.y;
			const dvec2 X = P + v * (B*T - A*S) / (A*A + B*B);

			const double det = determinant(v2 - P, Q - P);

			// constexpr uint32_t ray_color = 0x888888;
			constexpr uint32_t ray_color = 0xffffff;
			if (det < 0)
				draw_ray(bitmap, Q, X, ray_color);
			else {
				draw_ray(bitmap,
						Q,
						Q + (Q - X),
						ray_color);
			}
		} else {
			draw_edge(bitmap, tri_circumcenter[i/3],
					tri_circumcenter[d.halfedges[i]/3], 0xffffff);
			// draw_edge(bitmap, tri_centroid[i/3],
			// 		tri_centroid[d.halfedges[i]/3], 0xffffff);
		}
	}
	return;

	for (std::size_t i = 0; i < d.triangles.size(); ++i) {
		const std::size_t voronoi_id = d.triangles[i];
		if (voronoi_complete[voronoi_id])
			continue;
		voronoi_complete[voronoi_id] = true;

		// const std::size_t beg_triangle = i / 3;
		// triangles_of_voronoi[voronoi_id].push_back(beg_triangle);

		const std::size_t start_half_edge = i;
		std::size_t beg_half_edge = start_half_edge;
		do {
			const std::size_t next_triangle_half_edge
				= beg_half_edge - beg_half_edge%3 + (beg_half_edge+2)%3;;
			const std::size_t prev_voronoi_half_edge
				= d.halfedges[next_triangle_half_edge];
			if (prev_voronoi_half_edge == delaunator::INVALID_INDEX)
				break;

			beg_half_edge = prev_voronoi_half_edge;
		} while (beg_half_edge != start_half_edge);

		std::size_t cur_half_edge = beg_half_edge;
		do {
			const std::size_t cur_triangle = cur_half_edge/3;
			triangles_of_voronoi[voronoi_id].push_back(cur_triangle);

			const std::size_t twin_half_edge
				= d.halfedges[cur_half_edge];
			if (twin_half_edge == delaunator::INVALID_INDEX)
				break;
			cur_half_edge
				= twin_half_edge - twin_half_edge%3 + (twin_half_edge+1)%3;
		} while (cur_half_edge != start_half_edge);
	}

	// printf("\n");
	// for (std::size_t i = 0; i < 1; ++i)
	for (std::size_t i = 0; i < voronoi_cnt; ++i) {
		// draw_point(bitmap, dvec2(coords[2*i], coords[2*i+1]), 0.01f, 0xff0000);
		PRINT_ZU(triangles_of_voronoi[i].size());
		for (std::size_t j = 0; j < triangles_of_voronoi[i].size(); ++j) {
			const std::size_t a = triangles_of_voronoi[i][j];
			const std::size_t b = triangles_of_voronoi[i]
				[(j+1)%triangles_of_voronoi[i].size()];
			const dvec2 A = tri_circumcenter[a];
			const dvec2 B = tri_circumcenter[b];
			// const dvec2 A = tri_centroid[a];
			// const dvec2 B = tri_centroid[b];
			draw_edge(bitmap, A, B, 0xffffff);
			// PRINT_F(A.x);
		}
	}
}
