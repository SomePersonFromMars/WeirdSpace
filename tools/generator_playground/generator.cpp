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
using namespace glm;

#include "useful.hpp"
#include "settings.hpp"

#include "noise.hpp"
#include <delaunator.hpp>

generator_t::generator_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
	,ratio_wh{double(width)/double(height)}
	,ratio_hw{double(height)/double(width)}
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

	dvec3 val_v3 = lerp<dvec3, double>(
		// color_hex_to_vec3(0x826348),
		color_hex_to_vec3(0x826948),

		color_hex_to_vec3(0xf9d186),
		val);

	// dvec3 val_v3(1);

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

void generator_t::draw_edge(bitmap_t &bitmap, dvec2 beg, dvec2 end,
		uint32_t color, bool draw_only_empty) {
	beg = space_to_bitmap_coords(beg);
	end = space_to_bitmap_coords(end);

	dvec2 off = end - beg;
	const int iterations_cnt = std::max(
			std::abs(int(off.x)), std::abs(int(off.y)))+1;
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
		if (!draw_only_empty || bitmap.get(pos.y, pos.x) == 0x0)
			bitmap.set(pos.y, pos.x, color);
	}
}

void generator_t::draw_point(bitmap_t &bitmap, glm::dvec2 pos, double dim,
		uint32_t color) {

	ivec2 beg, end;
	beg.x = (pos.x-dim/2.0f)*double(width-1)/ratio_wh;
	end.x = (pos.x+dim/2.0f)*double(width-1)/ratio_wh;
	beg.y = (pos.y-dim/2.0f)*double(height-1);
	end.y = (pos.y+dim/2.0f)*double(height-1);

	for (int x = beg.x; x <= end.x; ++x) {
		if (x < 0 || x >= width) continue;

		for (int y = beg.y; y <= end.y; ++y) {
			if (y < 0 || y >= height) continue;
			bitmap.set(y, x, color);
		}
	}
}

void generator_t::fill(bitmap_t &bitmap, glm::dvec2 origin,
		uint32_t fill_color) {
	origin = space_to_bitmap_coords(origin);

	ivec2 first_pixel(origin.x, origin.y);
	// first_pixel.x = origin.x * double(width-1) / ratio_wh;
	// first_pixel.y = origin.y * double(height-1);
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
			if (cur_color != 0x000000)
				continue;

			next_pixel.push(p);
			bitmap.set(p.y, p.x, fill_color);
		}
	}
}

void generator_t::draw_convex_polygon(bitmap_t &bitmap,
		const std::vector<glm::dvec2> _points,
		const uint32_t color) {
	if (_points.size() == 0)
		return;
	using ll = long long;
	constexpr ll MULT = 10000;
	static std::vector<tvec2<ll, highp>> points;
	points.resize(_points.size());
	ll min_x = std::numeric_limits<ll>::max();
	ll min_y = std::numeric_limits<ll>::max();
	ll max_x = std::numeric_limits<ll>::min();
	ll max_y = std::numeric_limits<ll>::min();
	for (std::size_t i = 0; i < _points.size(); ++i) {
		points[i] = space_to_bitmap_coords(_points[i]);
		min_replace(min_x, points[i].x);
		max_replace(max_x, points[i].x);
		min_replace(min_y, points[i].y);
		max_replace(max_y, points[i].y);
		points[i] = space_to_bitmap_coords(_points[i])
			* static_cast<double>(MULT);
	}

	for (ll y = min_y; y <= max_y; ++y) {
		for (ll x = min_x; x <= max_x; ++x) {
			const tvec2<ll, highp> p(x*MULT, y*MULT);
			bool inside = true;
			for (std::size_t i = 0; i < points.size(); ++i) {
				const std::size_t j = i == points.size()-1 ? 0 : i+1;
				const ll det
					= determinant(points[j]-points[i], p-points[i]);
				if (!(det >= 0)) {
					inside = false;
					break;
				}
			}
			if (inside && bitmap.get(y, x) == 0x0)
				bitmap.set(y, x, color);
		}
	}
}

// void generator_t::draw_convex_polygon(bitmap_t &bitmap,
// 		const std::vector<glm::dvec2> _points,
// 		const uint32_t color) {
// 	if (_points.size() == 0)
// 		return;
// 	static std::vector<dvec2> points;
// 	points.resize(_points.size());
// 	double min_x = std::numeric_limits<double>::max();
// 	double min_y = std::numeric_limits<double>::max();
// 	double max_x = std::numeric_limits<double>::min();
// 	double max_y = std::numeric_limits<double>::min();
// 	for (std::size_t i = 0; i < _points.size(); ++i) {
// 		points[i] = space_to_bitmap_coords(_points[i]);
// 		min_replace(min_x, points[i].x);
// 		max_replace(max_x, points[i].x);
// 		min_replace(min_y, points[i].y);
// 		max_replace(max_y, points[i].y);
// 	}
//
// 	for (int y = min_y; y <= max_y; ++y) {
// 		for (int x = min_x; x <= max_x; ++x) {
// 			const dvec2 p(x, y);
// 			bool inside = true;
// 			for (std::size_t i = 0; i < points.size(); ++i) {
// 				const std::size_t j = i == points.size()-1 ? 0 : i+1;
// 				const double det
// 					= determinant(points[j]-points[i], p-points[i]);
// 				if (!(det >= 0)) {
// 					inside = false;
// 					break;
// 				}
// 			}
// 			if (inside)
// 				bitmap.set(y, x, color);
// 		}
// 	}
// }

void generator_t::draw_noisy_edge(bitmap_t &bitmap,
		std::mt19937 &gen,
		const std::size_t level,
		const double amplitude,
		const glm::dvec2 A,
		const glm::dvec2 B,
		const glm::dvec2 X,
		const glm::dvec2 Y,
		const uint32_t color) {
	if (level == 0) {
		draw_edge(bitmap, A, B, color);
	} else {
		std::uniform_real_distribution<double> distrib(
				0.5-amplitude, 0.5+amplitude);
		const dvec2 P = lerp(X, Y, distrib(gen));
		// const dvec2 P = lerp(X, Y, 1.0);
		draw_noisy_edge(
				bitmap,
				gen,

				level-1,
				amplitude,

				A, P,
				(A+X)/2.0,
				(A+Y)/2.0,
				color
				);
		draw_noisy_edge(
				bitmap,
				gen,

				level-1,
				amplitude,

				P, B,
				(B+X)/2.0,
				(B+Y)/2.0,
				color
				);
	}
}


void generator_B_t::fill(bitmap_t &bitmap, glm::dvec2 origin,
		uint32_t edge_color, uint32_t fill_color) {
	ivec2 first_pixel;
	first_pixel.x = origin.x * double(width-1) / ratio_wh;
	first_pixel.y = origin.y * double(height-1);
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
				const double depth = std::sqrt(
						double(len_sq(v - tile.coast_origin))) /
					double(tile_t::COAST_DEPTH+1);
				// const double depth = 1.0f -
				// 	double(tile.coast_dist) / double(tile_t::COAST_DEPTH);
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
	std::uniform_real_distribution<double> distrib_land(0.0, 1.0f);

	grid.assign(size.y, std::vector<tile_t>(size.x, {0}));

	std::queue<ivec2> next_v_A, next_v_B;
	std::set<ivec2, vec2_cmp_t<int>> points;
	for (int i = 0; i < voronois_cnt; ++i) {
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
		// 		double(i)/double(voronois_cnt), 0.9f, 0.8f);

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

void generator_C_t::generate_continents(std::mt19937 &gen) {
	std::uniform_real_distribution<double> distrib_x(0, space_max.x);
	std::uniform_real_distribution<double> distrib_y(0, space_max.y);
	diagram = voronoi_diagram_t();
	diagram.space_max = space_max;
	diagram.voronois.assign(600, voronoi_t());
	for (voronoi_t &voronoi : diagram.voronois) {
		voronoi.center.x = distrib_x(gen);
		voronoi.center.y = distrib_y(gen);
	}
	diagram.generate_relaxed(debug_val);
	// diagram.generate_relaxed(0);

	std::vector<std::size_t> super_voro_rep(
			diagram.voronois_cnt(), INVALID_ID);
	std::vector<double> super_voro_rep_dist(
			diagram.voronois_cnt(), std::numeric_limits<double>::infinity()
			);
	// Voronoi id, voronoi dist to its super voronoi representant
	using voro_priority_queue_t = std::pair<std::size_t, double>;
	auto cmp = []
		(const voro_priority_queue_t a, const voro_priority_queue_t b) {
			return a.second < b.second;
	};
	std::priority_queue<voro_priority_queue_t,
		std::vector<voro_priority_queue_t>, decltype(cmp)>
		next_v(cmp);
	std::set<std::size_t> super_voro_origins;

	std::uniform_int_distribution<std::size_t> distrib_voronoi_id(
			0, diagram.voronois_cnt()-1);
	while (super_voro_origins.size() < super_voro_cnt) {
		const std::size_t new_origin = distrib_voronoi_id(gen);
		const auto r = super_voro_origins.insert(new_origin);
		if (r.second) {
			super_voro_rep[new_origin] = new_origin;
			super_voro_rep_dist[new_origin] = 0.0;
			next_v.push(voro_priority_queue_t(new_origin, 0.0));
		}
	}

	while (!next_v.empty()) {
		const voro_priority_queue_t p = next_v.top();
		const std::size_t v = p.first;
		next_v.pop();
		const double v_dist_sq
			= len_sq(
					diagram.voronois[v].center -
					diagram.voronois[super_voro_rep[v]].center
					);
		if (v_dist_sq != p.second)
			continue;
		for (const voronoi_t::edge_t &neighbor : diagram.voronois[v].al) {
			const std::size_t w = neighbor.neighbor_id;
			const double cur_w_dist_sq
				= super_voro_rep[w] == INVALID_ID ?
				std::numeric_limits<double>::infinity() :
				len_sq(
						diagram.voronois[w].center -
						diagram.voronois[super_voro_rep[w]].center
						);
			const double new_w_dist_sq
				= len_sq(
						diagram.voronois[w].center -
						diagram.voronois[super_voro_rep[v]].center
						);
			if (new_w_dist_sq >= cur_w_dist_sq)
				continue;
			super_voro_rep[w] = super_voro_rep[v];
			super_voro_rep_dist[w] = new_w_dist_sq;
			next_v.push(
					voro_priority_queue_t(w, new_w_dist_sq));
		}
	}

	plates.assign(diagram.voronois_cnt(), plate_t());
	for (std::size_t i = 0, j = 0; i < diagram.voronois_cnt(); ++i) {
		if (super_voro_rep[i] != i)
			continue;
		plate_t &plate = plates[i];

		if (j <= super_voro_cnt * 25 / 100)
			plate.type = plate_t::LAND;
		else
			plate.type = plate_t::WATER;

		++j;
	}

	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		if (super_voro_rep[i] == i)
			continue;
		assert(super_voro_rep[i] != INVALID_ID);
		plates[i].type = plates[super_voro_rep[i]].type;
	}
}

void generator_C_t::draw_map(bitmap_t &bitmap, std::mt19937 &gen) {
	// for (const voronoi_t &voronoi : diagram.voronois) {
	// 	for (std::size_t j = 0; j < voronoi.points.size(); ++j) {
	// 		draw_edge(bitmap,
	// 				voronoi.points[j],
	// 				voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1],
	// 				0x777777);
	// 	}
	// }

	auto draw_voronoi = [&] (const std::size_t id) {
		const voronoi_t &voronoi = diagram.voronois[id];

		const uint32_t color = COLORS[static_cast<uint8_t>(plates[id].type)];
		// draw_convex_polygon(bitmap, voronoi.points, color);

		// for (std::size_t j = 0; j < voronoi.points.size(); ++j) {
		// 	draw_edge(bitmap,
		// 			voronoi.points[j],
		// 			voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1],
		// 			0x777777, true);
		// }

		for (std::size_t j = 0; j < voronoi.al.size(); ++j) {
			// if (j != 2)
			// 	continue;
			// if (j != debug_val)
			// 	continue;

			const voronoi_t::edge_t &edge = voronoi.al[j];
			if (!edge.visible)
				continue;
			if (diagram.half_edge_drawn[edge.smaller_half_edge_id])
				continue;
			diagram.half_edge_drawn[edge.smaller_half_edge_id] = true;

			draw_noisy_edge(
					bitmap,
					gen,

					5,
					0.15,
					edge.beg,
					edge.end,
					edge.quad_top,
					edge.quad_bottom,
					// 0xffffff
					color
					);
		}
	};
	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		draw_voronoi(i);
	}
	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		const voronoi_t &voronoi = diagram.voronois[i];
		fill(bitmap, voronoi.center, COLORS[
				static_cast<uint8_t>(plates[i].type)]);
	}
}

void generator_C_t::draw_tour_path(bitmap_t &bitmap, std::mt19937 &gen) {
	std::vector<std::size_t> land_voronois;
	std::vector<std::size_t> water_voronois;
	for (std::size_t i = 0; i < plates.size(); ++i) {
		if (plates[i].type == plate_t::LAND)
			land_voronois.push_back(i);
		if (plates[i].type == plate_t::WATER)
			water_voronois.push_back(i);
	}
	const std::size_t land_path_points_cnt = land_voronois.size() * 20 / 100;
	const std::size_t water_path_points_cnt = std::min(
			land_path_points_cnt * 25 / 75,
			water_voronois.size()
			);
	const std::size_t path_points_cnt
		= land_path_points_cnt + water_path_points_cnt;

	std::vector<double> chosen_points_coords(path_points_cnt*2);
	for (std::size_t i = 0; i < path_points_cnt; ++i) {
		chosen_points_coords[2*i+0]
			= diagram.voronois[land_voronois[i]].center.x;
		chosen_points_coords[2*i+1]
			= diagram.voronois[land_voronois[i]].center.y;
	}
	for (std::size_t i = 0; i < water_path_points_cnt; ++i) {
		chosen_points_coords[2*i+0]
			= diagram.voronois[water_voronois[i]].center.x;
		chosen_points_coords[2*i+1]
			= diagram.voronois[water_voronois[i]].center.y;
	}

	for (std::size_t i = 0; i < chosen_points_coords.size(); i += 2) {
		draw_point(bitmap,
				dvec2(chosen_points_coords[i], chosen_points_coords[i+1]),
				0.01, 0xef6b81);
	}

	const delaunator::Delaunator d(chosen_points_coords);

	struct weighted_edge_t {
		std::size_t a, b;
		double len_sq;
	};
	std::vector<weighted_edge_t> all_edges;
	std::vector<std::vector<std::size_t>> al(path_points_cnt);

	for (std::size_t half_edge = 0;
			half_edge < d.triangles.size();
			++half_edge) {
		weighted_edge_t weighted_edge;
		const std::size_t p = weighted_edge.a = d.triangles[half_edge];
		const std::size_t next_half_edge
			= (half_edge % 3 == 2) ? half_edge - 2 : half_edge + 1;
		const std::size_t q = weighted_edge.b = d.triangles[next_half_edge];

		if (p > q && d.halfedges[half_edge] != delaunator::INVALID_INDEX)
			continue;

		weighted_edge.len_sq
			= len_sq(
			dvec2(chosen_points_coords[2*q+0], chosen_points_coords[2*q+1]) -
			dvec2(chosen_points_coords[2*p+0], chosen_points_coords[2*p+1])
			);

		all_edges.push_back(weighted_edge);
	}

	std::sort(all_edges.begin(), all_edges.end(),
			[] (const weighted_edge_t &a, const weighted_edge_t &b) -> bool {
				return a.len_sq < b.len_sq;
			});

	struct fau_t {
		std::vector<std::size_t> rep, set_size;
		fau_t(const std::size_t size)
			:rep(size)
			,set_size(size, 1)
		{
			for (std::size_t i = 0; i < size; ++i)
				rep[i] = i;
		}

		std::size_t find(const std::size_t v) {
			if (rep[v] == v)
				return v;
			else
				return rep[v] = find(rep[v]);
		}

		void union_(std::size_t a, std::size_t b) {
			a = find(a);
			b = find(b);
			if (a == b)
				return;
			if (set_size[a] > set_size[b])
				std::swap(a, b);
			set_size[b] += set_size[a];
			rep[a] = b;
		}
	} fau(path_points_cnt);

	for (const weighted_edge_t &e : all_edges) {
		if (fau.find(e.a) == fau.find(e.b))
			continue;

		fau.union_(e.a, e.b);
		al[e.a].push_back(e.b);
		// al[e.b].push_back(e.a);
	}

	for (std::size_t v = 0; v < path_points_cnt; ++v) {
		for (const std::size_t w : al[v]) {
			draw_edge(bitmap,
				dvec2(
					chosen_points_coords[2*v+0],
					chosen_points_coords[2*v+1]),
				dvec2(
					chosen_points_coords[2*w+0],
					chosen_points_coords[2*w+1]),
				0x9e0922
				);
		}
	}
}

void generator_C_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {
	bitmap.clear();
	printf("\n");

	std::mt19937 gen(seed_voronoi);
	PRINT_LU(seed_voronoi);

	generate_continents(gen);
	draw_map(bitmap, gen);
	draw_tour_path(bitmap, gen);
}
