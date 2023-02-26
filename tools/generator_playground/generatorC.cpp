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
#include "geometry.hpp"
#include "settings.hpp"

#include "noise.hpp"
#include <delaunator.hpp>

generator_C_t::generator_C_t()
	:generator_t()
	,get_tour_path_point_x { [this] (const long long id) -> double {
		const double duplicate_off_x = diagram.space_max_x_duplicate_off;
		const auto [plane_id, point_id]
			= floor_div_rem(
					id,
					static_cast<long long>(tour_path_points.size()));
		return
			tour_path_points[point_id].x
			+ static_cast<double>(plane_id) * duplicate_off_x;
	} }
	,get_tour_path_point_y { [this] (const long long id) -> double {
		const long long point_id
			= floor_div_rem(
					id,
					static_cast<long long>(tour_path_points.size())).second;
		return tour_path_points[point_id].y;
	} }
	,GRID_HEIGHT{ceil_div(static_cast<size_t>(height), GRID_BOX_DIM_ZU)}
	,GRID_WIDTH{ceil_div(static_cast<size_t>(width/3), GRID_BOX_DIM_ZU)}
{
	noise.border_end = space_max.x*noise_pos_mult/3.0;
	noise.border_beg = noise.border_end;
	noise.border_beg -= CHUNK_DIM_F*noise_pos_mult*1.0;

	new_seed();
}

void generator_C_t::new_seed() {
	seed_voronoi = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1;

	noise.reseed(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1
	);
}

void generator_C_t::generate_continents(std::mt19937 &gen) {
	std::uniform_real_distribution<double> distrib_x(
			space_max.x * 1.0 / 3.0,
			space_max.x * 2.0 / 3.0);
	// std::uniform_real_distribution<double> distrib_x(0, space_max.x);
	std::uniform_real_distribution<double> distrib_y(0, space_max.y);
	diagram = voronoi_diagram_t();
	diagram.space_max = space_max;
	diagram.space_max_x_duplicate_off = space_max.x * 1.0 / 3.0;
	diagram.duplicate_off_vec = dvec2(diagram.space_max_x_duplicate_off, 0);
	diagram.voronois.assign(voro_cnt, voronoi_t());

	for (voronoi_t &voronoi : diagram.voronois) {
		voronoi.center.x = distrib_x(gen);
		voronoi.center.y = distrib_y(gen);
	}

	// for (std::size_t i = 0; i < voro_cnt/3; ++i) {
	// 	diagram.voronois[i].center.x = distrib_x(gen);
	// 	diagram.voronois[i].center.y = distrib_y(gen);

	// 	diagram.voronois[i+voro_cnt/3].center.x
	// 		= diagram.voronois[i].center.x - diagram.space_max_x_duplicate_off;
	// 	diagram.voronois[i+voro_cnt/3].center.y
	// 		= diagram.voronois[i].center.y;

	// 	diagram.voronois[i+voro_cnt*2/3].center.x
	// 		= diagram.voronois[i].center.x + diagram.space_max_x_duplicate_off;
	// 	diagram.voronois[i+voro_cnt*2/3].center.y
	// 		= diagram.voronois[i].center.y;
	// }

	diagram.generate_relaxed(debug_vals[0]);
	// diagram.generate_relaxed(3);

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

	std::uniform_int_distribution<uint32_t> color_distrib(
			1,
			std::numeric_limits<uint32_t>::max()
			);
	plates.assign(diagram.voronois_cnt(), plate_t());
	for (std::size_t i = 0, j = 0; i < diagram.voronois_cnt(); ++i) {
		plate_t &plate = plates[i];

		plate.debug_color = color_distrib(gen);

		if (super_voro_rep[i] != i)
			continue;

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

void generator_C_t::generate_grid_intersections() {
	using ll = long long;
	grid.assign(GRID_HEIGHT,
		std::vector<std::vector<voro_id_t>>(GRID_WIDTH));
	for (std::size_t voronoi_id = 0;
			voronoi_id < diagram.voronois.size(); ++voronoi_id) {
		const voronoi_t &voronoi = diagram.voronois[voronoi_id];
		const std::vector<dvec2> &points = voronoi.points;

		double min_x = std::numeric_limits<double>::max();
		double min_y = std::numeric_limits<double>::max();
		double max_x = std::numeric_limits<double>::min();
		double max_y = std::numeric_limits<double>::min();
		for (std::size_t i = 0; i < points.size(); ++i) {
			min_replace(min_x, points[i].x);
			max_replace(max_x, points[i].x);
			min_replace(min_y, points[i].y);
			max_replace(max_y, points[i].y);
		}

		tvec2<ll, highp> min_grid, max_grid;
		const auto space_to_grid_coords
			= [this] (const dvec2 &p) {
				return tvec2<ll, highp>(
						std::floor(
							(p.x-space_max.x/3.0) / GRID_BOX_DIM_F),
						std::floor(p.y / GRID_BOX_DIM_F)
					);
			};
		const auto calc_grid_constrains
			= [
				this,
				&space_to_grid_coords,
				&min_grid, &max_grid,
				&min_x, &max_x, &min_y, &max_y
			] {
				min_grid = space_to_grid_coords({min_x, min_y});
				max_grid = space_to_grid_coords({max_x, max_y});
				min_grid.x = clamp<ll>(min_grid.x, 0, GRID_WIDTH-1);
				max_grid.x = clamp<ll>(max_grid.x, 0, GRID_WIDTH-1);
				min_grid.y = clamp<ll>(min_grid.y, 0, GRID_HEIGHT-1);
				max_grid.y = clamp<ll>(max_grid.y, 0, GRID_HEIGHT-1);
			};

		calc_grid_constrains();
		for (ll y = min_grid.y; y <= max_grid.y; ++y) {
			for (ll x = min_grid.x; x <= max_grid.x; ++x) {
				grid[y][x].push_back({voronoi_id, voro_id_t::BASE});
			}
		}

		if (max_x > space_max.x*2.0/3.0) {
			max_replace(min_x, space_max.x*2.0/3.0);
			min_x -= space_max.x*1.0/3.0;
			max_x -= space_max.x*1.0/3.0;
			calc_grid_constrains();
			for (ll y = min_grid.y; y <= max_grid.y; ++y) {
				for (ll x = min_grid.x; x <= max_grid.x; ++x) {
					grid[y][x].push_back({voronoi_id, voro_id_t::LEFT});
				}
			}
		} else if (min_x < space_max.x*1.0/3.0) {
			min_replace(max_x, space_max.x*1.0/3.0);
			min_x += space_max.x*1.0/3.0;
			max_x += space_max.x*1.0/3.0;
			calc_grid_constrains();
			for (ll y = min_grid.y; y <= max_grid.y; ++y) {
				for (ll x = min_grid.x; x <= max_grid.x; ++x) {
					grid[y][x].push_back({voronoi_id, voro_id_t::RIGHT});
				}
			}
		}
	}

}

void generator_C_t::draw_map(bitmap_t &bitmap, std::mt19937 &gen) {
#define DRAW_GRID
#ifdef DRAW_GRID
	for (double x = 0; x <= space_max.x / 3.0; x += GRID_BOX_DIM_F) {
		draw_edge(bitmap,
				dvec2(x, 0),
				dvec2(x, space_max.y),
				0x424242);
		draw_edge(bitmap,
				dvec2(x+space_max.x/3.0, 0),
				dvec2(x+space_max.x/3.0, space_max.y),
				0x424242);
		draw_edge(bitmap,
				dvec2(x+space_max.x*2/3.0, 0),
				dvec2(x+space_max.x*2/3.0, space_max.y),
				0x424242);
	}
	for (double y = 0; y < space_max.y; y += GRID_BOX_DIM_F) {
		draw_edge(bitmap,
				dvec2(0, y),
				dvec2(space_max.x, y),
				0x424242);
	}
#endif

// #define DRAW_RELATIONS
#ifdef DRAW_RELATIONS
	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		const dvec2 A = diagram.voronois[i].center;
		for (const voronoi_t::edge_t &edge : diagram.voronois[i].al) {
			if (not edge.visible)
				continue;
			dvec2 B = diagram.voronois[edge.neighbor_id].center;
			if (edge.type == edge.TO_LEFT) {
				draw_edge(bitmap,
						A,
						B diagram.duplicate_off_vec,
						0xfcf485);
			} else if (edge.type == edge.TO_RIGHT) {
				draw_edge(bitmap,
						A,
						B + diagram.duplicate_off_vec,
						0xfcb985);
			} else {
				draw_edge(bitmap,
						A,
						B,
						0x70a9f9);
			}
		}
	}
#endif
#undef DRAW_RELATIONS

#define PSEUDO_PARALLEL_FILL
#ifdef PSEUDO_PARALLEL_FILL
	for (std::size_t y = 0; y < static_cast<std::size_t>(height); ++y) {
		for (std::size_t x = 0; x < static_cast<std::size_t>(width) / 3; ++x) {
			const dvec2 p = bitmap_to_space_coords(dvec2(x+width/3, y));
			const std::size_t x_grid = x / GRID_BOX_DIM_ZU;
			const std::size_t y_grid = y / GRID_BOX_DIM_ZU;
			const std::vector<voro_id_t> vec
				= grid[y_grid][x_grid];
			if (vec.size() == 0)
				continue;
			assert(vec.size() > 0);

			voro_id_t closest_voro_id;
			double closest_dist_sq = std::numeric_limits<double>::max();
			dvec2 closest_voro_center;
			for (std::size_t i = 0; i < vec.size(); ++i) {
				const voro_id_t voro_id = vec[i];
				const dvec2 voro_center
					= diagram.voronois[voro_id.id].center
					+ (
						vec[i].type == voro_id_t::LEFT ?
						-diagram.duplicate_off_vec :
						vec[i].type == voro_id_t::RIGHT ?
						diagram.duplicate_off_vec :
						dvec2(0)
					);

				const double new_dist_sq
					= len_sq(p - voro_center);
				if (new_dist_sq < closest_dist_sq) {
					closest_voro_id = voro_id;
					closest_dist_sq = new_dist_sq;
					closest_voro_center = voro_center;
				}
			}
			const voronoi_t &voro = diagram.voronois[closest_voro_id.id];
			const double closest_voronoi_dist_sq = closest_dist_sq;
			const dvec2 closest_voro_off
				= (
					closest_voro_id.type == voro_id_t::LEFT ?
					-diagram.duplicate_off_vec :
					closest_voro_id.type == voro_id_t::RIGHT ?
					diagram.duplicate_off_vec :
					dvec2(0)
				);

			std::size_t triangle_neighbor_edge_id = INVALID_ID;
			for (std::size_t i = 0;
					i < voro.al.size(); ++i) {
				const voronoi_t::edge_t &edge
					= voro.al[i];

				// beg <- p <- end
				const dvec2 A
					= (edge.beg + closest_voro_off) - closest_voro_center;
				const dvec2 B = p - closest_voro_center;
				const dvec2 C
					= (edge.end + closest_voro_off) - closest_voro_center;
				const double det1 = determinant(C, B);
				const double det2 = determinant(B, A);

				if (det1 <= 0 && det2 <= 0) {
					triangle_neighbor_edge_id = i;
				}
			}
			assert(voro.clipped || triangle_neighbor_edge_id != INVALID_ID);

			// Calculating elevation
			plate_t::type_t pixel_type = plates[closest_voro_id.id].type;
			double elevation = 0.0;
			if (triangle_neighbor_edge_id != INVALID_ID) {
				const voronoi_t::edge_t edge
					= voro.al[triangle_neighbor_edge_id];
				std::size_t triangle_neighbor_id = edge.neighbor_id;

				const auto [casted_edge_point, casted_edge_point_exists]
					= intersect_lines(
							p, closest_voro_center,
							edge.beg + closest_voro_off,
							edge.end + closest_voro_off);

				const double casted_point_dist
					= casted_edge_point_exists ?
					std::sqrt(len_sq(p - casted_edge_point)) :
					1.0;
				const double closest_voro_dist
					= std::sqrt(closest_voronoi_dist_sq);

				plate_t::type_t prev_type = plate_t::NONE;
				plate_t::type_t nxt_type  = plate_t::NONE;
				{
					std::size_t prev = triangle_neighbor_edge_id;
					std::size_t nxt = triangle_neighbor_edge_id;
					if (triangle_neighbor_edge_id == INVALID_ID) {
						prev = voro.al.size()-1;
						nxt = 0;
					} else {
						if (prev == 0) {
							if (voro.clipped) prev = INVALID_ID;
							else prev = voro.al.size()-1;
						} else
							--prev;
						if (nxt == voro.al.size()-1) {
							if (voro.clipped) nxt = INVALID_ID;
							else nxt = 0;
						} else ++nxt;
					}

					if (prev != INVALID_ID)
						prev_type = plates[voro.al[prev].neighbor_id].type;
					if (nxt != INVALID_ID)
						nxt_type = plates[voro.al[nxt].neighbor_id].type;
				}

				if (plates[closest_voro_id.id].type
					!= plates[triangle_neighbor_id].type
					// && plates[closest_voro_id.id].type == plate_t::LAND
				) {
					elevation
						= casted_point_dist
						/ (casted_point_dist + closest_voro_dist);
					if (elevation <= 1.0)
						pixel_type = plate_t::COAST;
				} else if (
					(prev_type != plate_t::NONE || nxt_type != plate_t::NONE)
					// && plates[closest_voro_id.id].type == plate_t::LAND
				) {
					const double det = determinant(
						edge.to_mid,
						p - closest_voro_center);
					elevation
						= 1.0 - std::abs(det) * 2.0
						/ edge.to_mid_len / edge.voro_edge_len;
					if (
						(
							prev_type != plate_t::NONE &&
							plates[closest_voro_id.id].type
							!= prev_type &&
							det <= 0
						) ||
						(
							nxt_type != plate_t::NONE &&
							plates[closest_voro_id.id].type
							!= nxt_type &&
							det >= 0
						)
					)
						pixel_type = plate_t::COAST;
				}

				if (plates[closest_voro_id.id].type == plate_t::LAND) {
					elevation /= 2.0;
					elevation += 0.5;
				} else {
					elevation = 1.0 - elevation;
					elevation /= 2.0;
				}
				if (pixel_type == plate_t::COAST) {
					constexpr double A = 0.2;
					constexpr double B = 0.9;
					if (elevation <= A)
						pixel_type = plate_t::WATER;
					if (elevation >= B)
						pixel_type = plate_t::LAND;
					elevation -= A;
					elevation /= (B-A);
				}

				// if (debug_vals[1] % 2 == 1) {
				// 	pixel_type = plate_t::COAST;
				// 	elevation
				// 		= casted_point_dist
				// 		/ (casted_point_dist + closest_voro_dist);
				// }
			} else {
				elevation = 0;
			}

			if (pixel_type == plate_t::WATER)
				elevation = 0;
			else if (pixel_type == plate_t::LAND)
				elevation = 1;

			uint32_t color;
			double noise_val = noise.octave2D_01(
				(p.x-space_max.x/3) * noise_pos_mult,
				p.y * noise_pos_mult,
				16);
			// noise_val *= 0.5;
			// noise_val += 0.5;
			// elevation *= noise_val;

			elevation = 0.2 + 0.8*elevation;
			{
				const double x = elevation;
				const double xx = x*x;
				const double xxx = xx*x;
				elevation = (-3.0*xxx + 4.0*xx + x) / 2.0;

				// elevation = 1.0 - elevation;
				// elevation = elevation*elevation ; // *elevation;
				// elevation = 1.0 - elevation;
			}
			elevation = elevation * 0.5 * (1.0 + noise_val);

			{
				assert(in_between_inclusive(0.0, 1.0, elevation));

				color = lerp(0, 0xff, elevation);
				// color = lerp(0xff, 0, elevation);
				color = color | (color << 8) | (color << 16);

				color = hsv_to_rgb(
					elevation * 240.0 / 360.0, 0.6, 0.8);

				// color = hsv_to_rgb(
				// 	debug_vals[0]*60.0 / 360.0, 0.5, 0.8);
			}

			// if (pixel_type == plate_t::COAST) {
			// 	color = lerp(0xff, 0, elevation);
			// 	color = color | (color << 8) | (color << 16);
			// 	// color = COLORS[plate_t::COAST];
			// } else {
			// 	if (pixel_type == plate_t::WATER)
			// 		color = 0xffffff;
			// 	else
			// 		color = 0;
			// 	// color = COLORS[pixel_type];
			// }

			// // Quite nice looking mosaic
			// // (use 0 key to switch between two modes)
			// color = plates[
			// 	(debug_vals[1] % 2) ? triangle_neighbor_id : closest_voro_id.id
			// ].debug_color;

			if (debug_vals[1] % 2 == 1) {
				// PRINT_F(noise_val);
				color = lerp(0xff, 0, noise_val);
				color = color | (color << 8) | (color << 16);
			}

			bitmap.set(y, x+width*0/3, color);
			bitmap.set(y, x+width*1/3, color);
			bitmap.set(y, x+width*2/3, color);
		}
	}
#endif
#undef PSEUDO_PARALLEL_FILL

#define DRAW_POLYGONS
#ifdef DRAW_POLYGONS
	auto draw_voronoi = [&] (
			const std::size_t id, const uint32_t override_color = 0xffffff) {
		const voronoi_t &voronoi = diagram.voronois[id];
		// const uint32_t color
		// = COLORS[static_cast<uint8_t>(plates[id].type)];
		// const uint32_t color = 0x015e1f;
		// draw_convex_polygon(bitmap, voronoi.points, color);
// #define DRAW_NOISY
#ifndef DRAW_NOISY
		for (std::size_t j = 0; j < voronoi.points.size(); ++j) {
			// draw_edge(bitmap,
			// 		voronoi.points[j],
			// 		voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1],
			// 		// color, true);
			// 		override_color, false);

			draw_edge(bitmap,
					voronoi.points[j]
					- diagram.duplicate_off_vec,
					voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1]
					- diagram.duplicate_off_vec,
					// color, true);
					// 0xa2f9d9, false);
					override_color, false);

			draw_edge(bitmap,
					voronoi.points[j]
					+ diagram.duplicate_off_vec,
					voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1]
					+ diagram.duplicate_off_vec,
					// color, true);
					// 0xa2eef9, false);
					override_color, false);
		}
#else
		for (std::size_t j = 0; j < voronoi.al.size(); ++j) {
			const voronoi_t::edge_t &edge = voronoi.al[j];
			if (!edge.visible)
				continue;
			if (diagram.half_edge_drawn[edge.smaller_half_edge_id])
				continue;
			diagram.half_edge_drawn[edge.smaller_half_edge_id] = true;

			std::mt19937 gen_copy = gen;
			if (edge.type == edge.USUAL || edge.type == edge.TO_RIGHT)
				draw_noisy_edge(
						bitmap,
						gen_copy,

						5,
						0.15,
						edge.beg,
						edge.end,
						edge.quad_top,
						edge.quad_bottom,
						// 0xffffff
						color
						);

			// const dvec2 off_vec(diagram.space_max_x_duplicate_off, 0);
			gen_copy = gen;
			if (edge.type == edge.USUAL)
				draw_noisy_edge(
						bitmap,
						gen_copy,

						5,
						0.15,
						edge.beg + diagram.duplicate_off_vec,
						edge.end + diagram.duplicate_off_vec,
						edge.quad_top + diagram.duplicate_off_vec,
						edge.quad_bottom + diagram.duplicate_off_vec,
						// 0xffffff
						color
						);

			draw_noisy_edge(
					bitmap,
					gen,

					5,
					0.15,
					edge.beg - diagram.duplicate_off_vec,
					edge.end - diagram.duplicate_off_vec,
					edge.quad_top - diagram.duplicate_off_vec,
					edge.quad_bottom - diagram.duplicate_off_vec,
					// 0xffffff
					color
					);
		}
#endif
#undef DRAW_NOISY
	};
	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		draw_voronoi(i);
	}

	// const std::size_t gy = debug_vals[0]/GRID_WIDTH;
	// const std::size_t gx = debug_vals[0]%GRID_WIDTH;
	// PRINT_ZU(gy);
	// PRINT_ZU(gx);
	// PRINT_ZU(GRID_WIDTH);
	// for (const std::size_t i
	// 		: grid[gy][gx]) {
	// 	draw_voronoi(i);
	// }

	// draw_voronoi(debug_vals[0], 0);
#endif
#undef DRAW_POLYGONS

// #define FLOOD_FILL
#ifdef FLOOD_FILL
	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		const voronoi_t &voronoi = diagram.voronois[i];
		fill(bitmap, voronoi.center, COLORS[
				static_cast<uint8_t>(plates[i].type)]);

		fill(bitmap, voronoi.center + diagram.duplicate_off_vec, COLORS[
				static_cast<uint8_t>(plates[i].type)]);

		fill(bitmap, voronoi.center - diagram.duplicate_off_vec, COLORS[
				static_cast<uint8_t>(plates[i].type)]);
	}
#endif
#undef FLOOD_FILL

// #define DRAW_SPACE_CYCLIC_BORDER
#ifdef DRAW_SPACE_CYCLIC_BORDER
	draw_edge(bitmap,
			dvec2(space_max.x * 1.0 / 3.0, 0),
			dvec2(space_max.x * 1.0 / 3.0, space_max.y),
			0x7c7c7c);
	draw_edge(bitmap,
			dvec2(space_max.x * 2.0 / 3.0, 0),
			dvec2(space_max.x * 2.0 / 3.0, space_max.y),
			0x7c7c7c);
#endif
#undef DRAW_SPACE_CYCLIC_BORDER
}

void generator_C_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {
	bitmap.clear();
	printf("\n");

	std::mt19937 gen(seed_voronoi);
	PRINT_LU(seed_voronoi);

	generate_continents(gen);
	generate_grid_intersections();

	std::size_t avg_cnt = 0;
	std::size_t max_cnt = 0;
	for (size_t y = 0; y < GRID_HEIGHT; ++y) {
		for (size_t x = 0; x < GRID_WIDTH; ++x) {
			avg_cnt += grid[y][x].size();
			max_replace(max_cnt, grid[y][x].size());
		}
	}
	avg_cnt /= GRID_HEIGHT*GRID_WIDTH;
	PRINT_ZU(avg_cnt);
	PRINT_ZU(max_cnt);
	// for (const auto e : grid[1][0])
	// 	PRINT_ZU(e);

	draw_map(bitmap, gen);
	// draw_tour_path(bitmap, gen);
}
