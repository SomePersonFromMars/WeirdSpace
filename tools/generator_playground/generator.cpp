#include "generator.hpp"

#include <cstdio>
#include <cstdlib>
#include <functional>
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

std::pair<dvec2, bool> generator_C_t::intersect_line_h_segment(
		dvec2 P, dvec2 v,
		dvec2 S) {
	const double &A = v.x;
	const double &B = v.y;
	dvec2 X;
	X.y = S.y;
	X.x = P.x + A * (S.y - P.y) / B;
	const bool on_line = in_between_inclusive(0.0, S.x, X.x);
	return {X, on_line};
}

std::pair<dvec2, bool> generator_C_t::intersect_line_v_segment(
		dvec2 P, dvec2 v,
		dvec2 S) {
	const double &A = v.x;
	const double &B = v.y;
	dvec2 X;
	X.x = S.x;
	X.y = P.y + B * (S.x - P.x) / A;
	const bool on_line = in_between_inclusive(0.0, S.y, X.y);
	return {X, on_line};
}

std::pair<dvec2, generator_C_t::inters_t>
	generator_C_t::find_closest_box_intersection_directed_edge(dvec2 P,
		dvec2 v, dvec2 S) {

	const double v_len_sq = len_sq(v);

	std::pair<dvec2, bool> r;
	inters_t inters = inters_t::OUTSIDE;

	double closest_X_dist_sq = std::numeric_limits<double>::max();
	dvec2 closest_X;

	double dist_sq;
	dvec2 w;

	// Find intersection starting from P
	// Left box edge
	r = intersect_line_v_segment(P, v, {0, S.y});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::LEFT;
	}

	// Right box edge
	r = intersect_line_v_segment(P, v, {S.x, S.y});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::RIGHT;
	}

	// Bottom box edge
	r = intersect_line_h_segment(P, v, {S.x, 0});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::BOTTOM;
	}

	// Top box edge
	r = intersect_line_h_segment(P, v, {S.x, S.y});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::TOP;
	}

	if (inters == inters_t::OUTSIDE) {
		if (in_between_inclusive(0.0, S.x, P.x)
			&& in_between_inclusive(0.0, S.y, P.y))
			inters = inters_t::INSIDE;
	}

	return {closest_X, inters};
}

std::pair<generator_C_t::reduced_edge_t, bool> generator_C_t::trim_edge(
		glm::dvec2 beg, glm::dvec2 end, glm::dvec2 S
		) {

	// Return value
	reduced_edge_t red_edge;
	red_edge.beg = beg;
	red_edge.end = end;
	red_edge.beg_inters = inters_t::INSIDE;
	red_edge.end_inters = inters_t::INSIDE;

	std::pair<dvec2, inters_t> r;
	// Find intersection starting from beg
	r = find_closest_box_intersection_directed_edge(
			beg, end-beg, S);
	if (r.second == inters_t::OUTSIDE) {
		return {reduced_edge_t(), false};
	}
	if (
			r.second != inters_t::INSIDE &&
			(
				!in_between_inclusive(0.0, S.x, beg.x) ||
				!in_between_inclusive(0.0, S.y, beg.y)
			)
		) {
		red_edge.beg = r.first;
		red_edge.beg_inters = r.second;
	}

	// Find intersection starting from end
	r = find_closest_box_intersection_directed_edge(
			end, -(end-beg), S);
	if (
			r.second != inters_t::INSIDE &&
			(
				!in_between_inclusive(0.0, S.x, end.x) ||
				!in_between_inclusive(0.0, S.y, end.y)
			)
		) {
		red_edge.end = r.first;
		red_edge.end_inters = r.second;
	}

	return {red_edge, true};
}

std::pair<generator_C_t::reduced_edge_t, bool> generator_C_t::trim_inf_edge(
		const glm::dvec2 beg,
		const glm::dvec2 direction_vec,
		const glm::dvec2 S
		) {

	dvec2 closest_intersections[2];
	inters_t closest_intersections_type[2] {
		inters_t::OUTSIDE,
		inters_t::OUTSIDE
	};
	double closest_intersections_dist_sq[2] {
		std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::infinity()
	};

	std::pair<dvec2, bool> r;
	dvec2 w;
	double w_len_sq;

	// Find intersections starting from beg
	// Left box edge
	r = intersect_line_v_segment(beg, direction_vec, {0, S.y});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::LEFT;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::LEFT;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	// Right box edge
	r = intersect_line_v_segment(beg, direction_vec, {S.x, S.y});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::RIGHT;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::RIGHT;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	// Bottom box edge
	r = intersect_line_h_segment(beg, direction_vec, {S.x, 0});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::BOTTOM;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::BOTTOM;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	// Top box edge
	r = intersect_line_h_segment(beg, direction_vec, {S.x, S.y});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::TOP;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::TOP;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	if (closest_intersections_dist_sq[0]
			== std::numeric_limits<double>::infinity()) {
		return {reduced_edge_t(), false};
	} else if (closest_intersections_dist_sq[1]
			== std::numeric_limits<double>::infinity()) {
		reduced_edge_t red_edge;
		red_edge.beg = beg;
		red_edge.beg_inters = inters_t::INSIDE;
		red_edge.end = closest_intersections[0];
		red_edge.end_inters = closest_intersections_type[0];
		return {red_edge, true};
	} else {
		reduced_edge_t red_edge;
		red_edge.beg = closest_intersections[0];
		red_edge.beg_inters = closest_intersections_type[0];
		red_edge.end = closest_intersections[1];
		red_edge.end_inters = closest_intersections_type[1];
		return {red_edge, true};
	}
}

void generator_C_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {
	bitmap.clear();
	printf("\n");

	// // Coordinates tests
	// draw_point(bitmap, vec2(0.1f, 0.1f) * space_max, 0.01, 0xff0000);
	// draw_point(bitmap, vec2(0.9f, 0.1f) * space_max, 0.01, 0x00ff00);
	// draw_point(bitmap, vec2(0.1f, 0.9f) * space_max, 0.01, 0x0000ff);
	// draw_edge(bitmap, (vec2(0.0f, 0.0f)+vec2(0.5f))*space_max,
	// 		(vec2(-2.0f, 3.0f)+vec2(0.5f))*space_max, 0xff0000);
	// draw_edge(bitmap, (vec2(0.0f, 0.0f)+vec2(0.5f))*space_max,
	// 		(vec2(3.0f, 2.0f)+vec2(0.5f))*space_max, 0x00ff00);

	// Edge case seeds:
	// Non-infinite voronois clipping:
	// seed_voronoi = 1667157016119;
	// seed_voronoi = 1667156960118;

	std::mt19937 gen(seed_voronoi);
	PRINT_LU(seed_voronoi);
	std::uniform_real_distribution<double> distrib_x(0,
			double(width-1)/double(width) * ratio_wh);
	std::uniform_real_distribution<double> distrib_y(0,
			double(height-1)/double(height));
	// std::uniform_real_distribution<double> distrib_x(0.2,
	// 		double(width-1)/double(width) * ratio_wh -0.2);
	// std::uniform_real_distribution<double> distrib_y(0.2,
	// 		double(height-1)/double(height) -0.2);

	constexpr std::size_t voronoi_cnt = 60;
	// constexpr std::size_t voronoi_cnt = 5;
	std::vector<double> coords(voronoi_cnt*2);
	for (std::size_t i = 0; i < voronoi_cnt; ++i) {
		coords[2*i+0] = distrib_x(gen);
		coords[2*i+1] = distrib_y(gen);
	}

	delaunator::Delaunator d(coords);
	// // Mirroring the Y coordinates so the triangles are directed clockwise
	// // in OpenGL coordinate system. For now the algorithm is breaking with
	// // this change.
	// for (std::size_t i = 0; i < voronoi_cnt; ++i) {
	// 	coords[2*i+1] = 1.0-coords[2*i+1];
	// }

	std::vector<voronoi_t> voronois(voronoi_cnt);
	std::vector<dvec2> tri_centroid(d.triangles.size() / 3);
	std::vector<dvec2> tri_circumcenter(d.triangles.size() / 3);
	for (std::size_t i = 0; i < d.triangles.size() / 3; ++i) {
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

		draw_edge(bitmap, A, B, 0x011a8c);
		draw_edge(bitmap, A, C, 0x011a8c);
		draw_edge(bitmap, B, C, 0x011a8c);

		// // Coordinates test
		// if (i == 5) {
		// 	draw_point(bitmap, A, 0.01, 0xff0000);
		// 	draw_point(bitmap, B, 0.01, 0x00ff00);
		// 	draw_point(bitmap, C, 0.01, 0x0000ff);
		// }
	}

	// Iterate over all half edges
	for (std::size_t i = 0; i < d.triangles.size(); ++i) {
		voronoi_t &voronoi = voronois[d.triangles[i]];
		if (voronoi.complete)
			continue;
		voronoi.complete = true;

		// Finding the triangle (if such exists)
		// that is adjacent to void that is also
		// the most counter clockwise rotated about
		// the voronoi center.
		const std::size_t start_half_edge = i;
		std::size_t beg_half_edge = start_half_edge;
		do {
			const std::size_t next_half_edge_in_triangle
				= beg_half_edge - beg_half_edge%3 + (beg_half_edge+2)%3;
			const std::size_t prev_voronoi_half_edge
				= d.halfedges[next_half_edge_in_triangle];
			if (prev_voronoi_half_edge == delaunator::INVALID_INDEX) {
				voronoi.edge_voronoi = true;
				break;
			}

			beg_half_edge = prev_voronoi_half_edge;
		} while (beg_half_edge != start_half_edge);

		// Finding the triangle (if such exists)
		// that is adjacent to void that is also
		// the most clockwise rotated about
		// the voronoi center.
		// Also add all visited triangles' half edges
		// such that they have their beg (or end) in the voronoi center
		// as the current voronoi's half edges.
		if (voronoi.edge_voronoi) {
			// This is the only case that a half edge has its end
			// in the voronoi center.
			voronoi.tri_half_edges.push_back(
				beg_half_edge - beg_half_edge%3 + (beg_half_edge+2)%3
				);
		}
		std::size_t cur_half_edge = beg_half_edge;
		do {
			voronoi.tri_half_edges.push_back(cur_half_edge);

			const std::size_t twin_half_edge
				= d.halfedges[cur_half_edge];
			if (twin_half_edge == delaunator::INVALID_INDEX)
				break;
			cur_half_edge
				= twin_half_edge - twin_half_edge%3 + (twin_half_edge+1)%3;
		} while (cur_half_edge != beg_half_edge);
	}

	for (std::size_t i = 0; i < voronoi_cnt; ++i) {
		constexpr uint32_t color = 0x444444;
		// constexpr uint32_t color = 0xffffff;
		for (const std::size_t j : voronois[i].tri_half_edges) {
			if (d.halfedges[j] == delaunator::INVALID_INDEX) {
				const dvec2 v1(
						coords[2 * d.triangles[j]],
						coords[2 * d.triangles[j] + 1]);
				const dvec2 v2(
						coords[2 * d.triangles[j-j%3+(j+1)%3]],
						coords[2 * d.triangles[j-j%3+(j+1)%3] + 1]);
				const dvec2 &P = v1;
				const dvec2 &Q = tri_circumcenter[j/3];
				const dvec2 v = v2 - v1;
				const double &A = v.x;
				const double &B = v.y;
				if (A == 0 && B == 0)
					continue;
				const double S = P.x - Q.x;
				const double T = Q.y - P.y;
				const dvec2 X = P + v * (B*T - A*S) / (A*A + B*B);

				const double det = determinant(v2 - P, Q - P);

				const uint32_t ray_color = color;
				if (det < 0)
					draw_ray(bitmap, Q, X, ray_color);
				else {
					draw_ray(bitmap,
							Q,
							Q + (Q - X),
							ray_color);
				}
			} else {
				draw_edge(bitmap, tri_circumcenter[j/3],
						tri_circumcenter[d.halfedges[j]/3], color);
			}
		}
	}

	std::pair<reduced_edge_t, bool> tmp_r;
	// // 1
	// tmp_r = trim_edge(dvec2(3, -1), dvec2(-2, 6), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 2
	// tmp_r = trim_edge(dvec2(1, 6), dvec2(2, 4), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 3
	// tmp_r = trim_edge(dvec2(4, 2), dvec2(7, 4), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 4
	// tmp_r = trim_edge(dvec2(6, 1), dvec2(7, -1), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 5
	// tmp_r = trim_edge(dvec2(8, -4), dvec2(6, -2), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 6
	// tmp_r = trim_edge(dvec2(8, 2), dvec2(11, 5), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 7
	// tmp_r = trim_edge(dvec2(13, -3), dvec2(10, -1), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 8
	// tmp_r = trim_edge(dvec2(4, 0), dvec2(6, 0), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 9
	// tmp_r = trim_edge(dvec2(9, -1), dvec2(9, 1), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 10
	// tmp_r = trim_edge(dvec2(4, 6), dvec2(7, 6), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 11
	// tmp_r = trim_edge(dvec2(7, 7), dvec2(11, 7), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 1
	// tmp_r = trim_inf_edge(dvec2(-2, 5), dvec2(1, -2), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 2
	// tmp_r = trim_inf_edge(dvec2(2, 6), dvec2(1, -1), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 3
	// tmp_r = trim_inf_edge(dvec2(1, -3), dvec2(2, 0), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 4
	// tmp_r = trim_inf_edge(dvec2(-1, -4), dvec2(3, 0), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 5
	// tmp_r = trim_inf_edge(dvec2(6, 3), dvec2(7, 2), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 6
	// tmp_r = trim_inf_edge(dvec2(6, 2), dvec2(1, 0), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 7
	// tmp_r = trim_inf_edge(dvec2(9, 1), dvec2(4, -3), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// // 8
	// tmp_r = trim_inf_edge(dvec2(9, -2), dvec2(0, 1), dvec2(9, 5));
	// PRINT_D(static_cast<int>(tmp_r.second));
	// PRINT_VEC2(tmp_r.first.beg);
	// PRINT_VEC2(tmp_r.first.end);
	// PRINT_D(static_cast<int>(tmp_r.first.beg_inters));
	// PRINT_D(static_cast<int>(tmp_r.first.end_inters));
	// puts("");

	// std::size_t debug_cnter = 0;
	for (std::size_t i = 0; i < voronoi_cnt; ++i) {
		voronoi_t &voronoi = voronois[i];
		for (const std::size_t j : voronoi.tri_half_edges) {
			bool exists;
			reduced_edge_t red_edge;
			if (d.halfedges[j] == delaunator::INVALID_INDEX) {
				const dvec2 v1(
						coords[2 * d.triangles[j]],
						coords[2 * d.triangles[j] + 1]);
				const dvec2 v2(
						coords[2 * d.triangles[j-j%3+(j+1)%3]],
						coords[2 * d.triangles[j-j%3+(j+1)%3] + 1]);
				const dvec2 &beg = tri_circumcenter[j/3];
				const dvec2 v = v2 - v1;
				// Rotating the new edge 90 degrees relatively to
				// the current tri_half_edge.
				// Not sure why rotating counter clockwise instead of rotating
				// clockwise, but it seems to be working correctly. :)
				// It is probably related to the Delaunator coordinate system
				// problem.
				const dvec2 direction_vec(-v.y, v.x);
				const dvec2 S = space_max;

				const std::pair<reduced_edge_t, bool> r
					= trim_inf_edge(beg, direction_vec, S);
				red_edge = r.first;
				exists = r.second;
				if (r.second) {
					if (voronoi.red_edges.empty()) {
						// This is the only case that a half edge has its end
						// in the voronoi center, so it is necessary to swap
						// beg with end in the new edge.
						std::swap(red_edge.beg, red_edge.end);
						std::swap(red_edge.beg_inters, red_edge.end_inters);
						// voronoi.end_edge_inters = red_edge.beg_inters;
					}
					// else {
					// 	// This is nonfirst infinite edge, so
					// 	// it must be the last one.
					// 	// voronoi.beg_edge_inters = red_edge.end_inters;
					// }
					// voronoi.red_edges.push_back(red_edge);
					// voronoi.points.push_back(red_edge.beg);
					// voronoi.points.push_back(red_edge.end);
				}
			} else {
				const dvec2 beg = tri_circumcenter[j/3];
				const dvec2 end = tri_circumcenter[d.halfedges[j]/3];
				const dvec2 S = space_max;

				const std::pair<reduced_edge_t, bool> r
					= trim_edge(beg, end, S);
				red_edge = r.first;
				exists = r.second;
				// if (r.second) {
				// 	// if (
				// 	// 	r.first.beg_inters != inters_t::INSIDE ||
				// 	// 	r.first.end_inters != inters_t::INSIDE)
				// 	// 	voronoi.edge_voronoi = true;
				// 	// voronoi.red_edges.push_back(r.first);
				// 	// voronoi.points.push_back(r.first.beg);
				// 	// voronoi.points.push_back(r.first.end);
				// }
			}

			if (exists) {
				if (red_edge.end_inters != inters_t::INSIDE) {
					voronoi.last_incoming_red_edge_id
						= voronoi.red_edges.size();
					voronoi.edge_voronoi = true;
				}
				voronoi.red_edges.push_back(red_edge);
			}
		}

		if (voronoi.edge_voronoi) {
			std::size_t j = voronoi.last_incoming_red_edge_id;
			assert(j != INVALID_ID);

			do {
				const reduced_edge_t &e1 = voronoi.red_edges[j];
				voronoi.points.push_back(e1.beg);
				if (!(e1.end_inters != inters_t::INSIDE)) {
					continue;
				}
				voronoi.points.push_back(e1.end);
				const reduced_edge_t &e2
					= voronoi.red_edges[(j+1) % voronoi.red_edges.size()];
				assert(e2.beg_inters != inters_t::INSIDE);

				const inters_t beg_edge_inters = e1.end_inters;
				const inters_t end_edge_inters = e2.beg_inters;
				const uint8_t beg_side_id
					= static_cast<uint8_t>(beg_edge_inters);
				const uint8_t end_side_id
					= static_cast<uint8_t>(end_edge_inters);
				constexpr uint8_t SIDES_CNT
					= static_cast<uint8_t>(inters_t::SIDES_CNT);
				assert(beg_side_id < SIDES_CNT);
				assert(end_side_id < SIDES_CNT);

				if (beg_side_id != end_side_id)
					for (
							uint8_t k = (beg_side_id+0)%SIDES_CNT;
							k != end_side_id;
							k = (k+1)%SIDES_CNT) {
						switch (static_cast<inters_t>(k)) {
							case inters_t::LEFT:
								voronoi.points.push_back({0, 0});
								break;
							case inters_t::TOP:
								voronoi.points.push_back({0, space_max.y});
								break;
							case inters_t::RIGHT:
								voronoi.points.push_back(
										{space_max.x, space_max.y});
								break;
							case inters_t::BOTTOM:
								voronoi.points.push_back({space_max.x, 0});
								break;
							default:
								break;
						}
					}

			} while (
					j = (j+1) % voronoi.red_edges.size(),
					j != voronoi.last_incoming_red_edge_id
					);
		} else {
			for (const reduced_edge_t &e : voronoi.red_edges)
				voronoi.points.push_back(e.beg);
		}

		// if (false) {
		// 	// if (!voronoi.points.empty() && voronoi.edge_voronoi)
		// 	const reduced_edge_t &first
		// 		= voronoi.red_edges.front();
		// 	const reduced_edge_t &last
		// 		= voronoi.red_edges.back();
		// 	const inters_t beg_edge_inters = last.end_inters;
		// 	const inters_t end_edge_inters = first.beg_inters;

		// 	const uint8_t beg_side_id
		// 		= static_cast<uint8_t>(beg_edge_inters);
		// 	const uint8_t end_side_id
		// 		= static_cast<uint8_t>(end_edge_inters);
		// 	constexpr uint8_t SIDES_CNT
		// 		= static_cast<uint8_t>(inters_t::SIDES_CNT);
		// 	assert(beg_side_id < SIDES_CNT);
		// 	assert(end_side_id < SIDES_CNT);

		// 	// PRINT_D(beg_side_id);
		// 	// PRINT_D(end_side_id);

		// 	if (beg_side_id != end_side_id)
		// 		for (
		// 				uint8_t j = (beg_side_id+0)%SIDES_CNT;
		// 				j != end_side_id;
		// 				j = (j+1)%SIDES_CNT) {
		// 			switch (static_cast<inters_t>(j)) {
		// 				case inters_t::LEFT:
		// 					voronoi.points.push_back({0, 0});
		// 					break;
		// 				case inters_t::TOP:
		// 					voronoi.points.push_back({0, space_max.y});
		// 					break;
		// 				case inters_t::RIGHT:
		// 					voronoi.points.push_back(
		// 							{space_max.x, space_max.y});
		// 					break;
		// 				case inters_t::BOTTOM:
		// 					voronoi.points.push_back({space_max.x, 0});
		// 					break;
		// 				default:
		// 					break;
		// 			}
		// 		}
		// }
	}

	// std::size_t debug_cnter = 0;
	for (std::size_t i = 0; i < voronoi_cnt; ++i) {
		const voronoi_t &voronoi = voronois[i];
		// if (i != 26)
		// 	continue;
		// if (!voronoi.edge_voronoi)
		// 	continue;
		// if (debug_cnter++ != debug_val)
		// 	continue;
		constexpr uint32_t color = 0xffffff;
		// draw_point(bitmap, voronoi.points[0], 0.01f, 0x888800);
		// draw_point(bitmap, voronoi.points.back(), 0.01f, 0x880000);
		for (std::size_t j = 0; j < voronoi.points.size(); ++j) {
			draw_edge(bitmap,
					voronoi.points[j],
					voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1],
					color);
		}
		// for (const reduced_edge_t &red_edge : voronoi.red_edges) {
		// 	draw_edge(bitmap,
		// 			red_edge.beg,
		// 			red_edge.end,
		// 			color);
		// }
	}
}
