// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "map_generator.hpp"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#ifdef _MSC_VER
	#include <corecrt_math_defines.h>
#endif
#include <functional>
#include <ctime>
#include <chrono>
#include <random>
#include <vector>
#include <queue>
#include <set>

#include <glm/glm.hpp>
using namespace glm;

#include <useful.hpp>
#include <geometry.hpp>
#include <settings.hpp>

#include "noise.hpp"
#include <delaunator.hpp>

// Constructor
map_generator_t::map_generator_t(map_storage_t * const map_storage)
	:map_storage{map_storage}
	,width{map_storage->get_width()}
	,height{map_storage->get_height()}
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
{
	// new_seed();
}

void map_generator_t::load_settings() {
	voro_cnt = global_settings.voro_cnt;
	super_voro_cnt = std::min(voro_cnt, global_settings.super_voro_cnt);

	calculate_constants();
}

void map_generator_t::calculate_constants() {
	grid_box_dim_zu = global_settings.map_unit_resolution/4;

	ratio_wh = double(width)/double(height);
	ratio_hw = double(height)/double(width);
	space_max = {ratio_wh, 1};
	if (global_settings.triple_map_size)
		space_max.x /= 3.0;
	real_space_max = {space_max.x*3.0, space_max.y};
	space_max_x_duplicate_off = space_max.x * 1.0;
	space_max_duplicate_off_vec = {space_max_x_duplicate_off, 0};

	grid_height = ceil_div(static_cast<size_t>(height), grid_box_dim_zu);
	grid_width = ceil_div(static_cast<size_t>(width/3), grid_box_dim_zu);

	grid_box_dim_f
		= static_cast<double>(grid_box_dim_zu)
		* space_max.y / static_cast<double>(height);
	noise_pos_mult = 6.0;
	noise.border_end = space_max.x*noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= noise_pos_mult / 3.0;
}

void map_generator_t::new_seed() {
	auto nseed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 1;
	if (global_settings.replace_seed != 0)
		nseed = global_settings.replace_seed;

	seed_voronoi = nseed;
	noise.reseed(nseed);
}

void map_generator_t::generate_continents(std::mt19937 &gen) {
	std::uniform_real_distribution<double> distrib_x(
			space_max.x * 1.0,
			space_max.x * 2.0);
	// std::uniform_real_distribution<double> distrib_x(0, space_max.x);
	std::uniform_real_distribution<double> distrib_y(0, space_max.y);
	diagram = voronoi_diagram_t();
	diagram.space_max = real_space_max;
	diagram.space_max_x_duplicate_off = space_max_x_duplicate_off;
	diagram.duplicate_off_vec = space_max_duplicate_off_vec;
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

	diagram.generate_relaxed(8);
	// diagram.generate_relaxed(debug_vals[0]);
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

		if (j <= super_voro_cnt * global_settings.land_probability)
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

void map_generator_t::generate_grid_intersections() {
	using ll = long long;
	grid.assign(grid_height,
		std::vector<std::vector<voro_id_t>>(grid_width));
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
		const auto calc_grid_constrains
			= [
				this,
				&min_grid, &max_grid,
				&min_x, &max_x, &min_y, &max_y
			] {
				min_grid = space_to_grid_coords({min_x, min_y});
				max_grid = space_to_grid_coords({max_x, max_y});
				min_grid.x = clamp<ll>(min_grid.x, 0, grid_width-1);
				max_grid.x = clamp<ll>(max_grid.x, 0, grid_width-1);
				min_grid.y = clamp<ll>(min_grid.y, 0, grid_height-1);
				max_grid.y = clamp<ll>(max_grid.y, 0, grid_height-1);
			};

		calc_grid_constrains();
		for (ll y = min_grid.y; y <= max_grid.y; ++y) {
			for (ll x = min_grid.x; x <= max_grid.x; ++x) {
				grid[y][x].push_back({voronoi_id, voro_id_t::BASE});
			}
		}

		if (max_x > space_max.x*2.0) {
			max_replace(min_x, space_max.x*2.0);
			min_x -= space_max.x*1.0;
			max_x -= space_max.x*1.0;
			calc_grid_constrains();
			for (ll y = min_grid.y; y <= max_grid.y; ++y) {
				for (ll x = min_grid.x; x <= max_grid.x; ++x) {
					grid[y][x].push_back({voronoi_id, voro_id_t::LEFT});
				}
			}
		} else if (min_x < space_max.x*1.0) {
			min_replace(max_x, space_max.x*1.0);
			min_x += space_max.x*1.0;
			max_x += space_max.x*1.0;
			calc_grid_constrains();
			for (ll y = min_grid.y; y <= max_grid.y; ++y) {
				for (ll x = min_grid.x; x <= max_grid.x; ++x) {
					grid[y][x].push_back({voronoi_id, voro_id_t::RIGHT});
				}
			}
		}
	}
}

// Credits:
// https://github.com/dandrino/terrain-erosion-3-ways/tree/master?tab=readme-ov-file#river-networks
// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
void map_generator_t::generate_joints(
		std::mt19937 &gen) {
	using ll = long long;
	const double R = global_settings.river_joints_R;
	const double RR = R*R;
	constexpr double SQRT_2 = 1.4142135623730951;
	const double CELL_DIM = R / SQRT_2;
	const double BG_GRID_WIDTH_F = std::ceil(space_max.x / CELL_DIM);
	const double BG_GRID_HEIGHT_F = std::ceil(space_max.y / CELL_DIM);
	const std::size_t BG_GRID_WIDTH = BG_GRID_WIDTH_F;
	const std::size_t BG_GRID_HEIGHT = BG_GRID_HEIGHT_F;
	std::uniform_real_distribution<double> alpha_distrib(0.0, 2.0*M_PI);
	std::uniform_real_distribution<double> r_distrib(R, 2*R);

	joints.clear();
	joints.reserve(BG_GRID_WIDTH*BG_GRID_HEIGHT);
	std::vector<std::size_t> active_list;
	active_list.reserve(joints.capacity());
	std::vector<std::vector<std::size_t>> bg_grid(
		BG_GRID_HEIGHT, std::vector<std::size_t>(
			BG_GRID_WIDTH, INVALID_ID));

	const auto add_point = [&] (const dvec2 &p) -> bool {
		const dvec2 grid_p_f = p / CELL_DIM;
		const dvec2 grid_p_f_floor {
			std::floor(grid_p_f.x),
			std::floor(grid_p_f.y)
		};
		const tvec2<ll, highp> grid_p {
			(ll)grid_p_f_floor.x,
			(ll)grid_p_f_floor.y
		};
		if (
			not (p.x >= 0.0) ||
			not (p.x <= space_max.x) ||
			not (p.y >= 0.0) ||
			not (p.y <= space_max.y)
		) return false;
		assert(grid_p_f_floor.x >= 0.0);
		assert(grid_p_f_floor.x <= BG_GRID_WIDTH_F-1.0);
		assert(grid_p_f_floor.y >= 0.0);
		assert(grid_p_f_floor.y <= BG_GRID_HEIGHT_F-1.0);
		double min_x_f = grid_p_f_floor.x - 2.0;
		double min_y_f = grid_p_f_floor.y - 2.0;
		double max_x_f = grid_p_f_floor.x + 2.0;
		double max_y_f = grid_p_f_floor.y + 2.0;
		if (grid_p_f.x - grid_p_f_floor.x < 0.5)
			max_x_f -= 1.0;
		else min_x_f += 1.0;
		if (grid_p_f.y - grid_p_f_floor.y < 0.5)
			max_y_f -= 1.0;
		else min_y_f += 1.0;
		if (min_x_f < 0.0) min_x_f -= 1.0;
		if (max_x_f >= BG_GRID_WIDTH_F-1.0) max_x_f += 1.0;
		// const ll min_x
		// 	= (ll)clamp(min_x_f, 0.0, BG_GRID_WIDTH_F-1.0);
		const ll min_x = min_x_f;
		const ll min_y
			= (ll)clamp(min_y_f, 0.0, BG_GRID_HEIGHT_F-1.0);
		// const ll max_x
		// 	= (ll)clamp(max_x_f, 0.0, BG_GRID_WIDTH_F-1.0);
		const ll max_x = max_x_f;
		const ll max_y
			= (ll)clamp(max_y_f, 0.0, BG_GRID_HEIGHT_F-1.0);
		bool distant = true;
		for (ll x_ = min_x; x_ <= max_x && distant; ++x_) {
			for (ll y = min_y; y <= max_y && distant; ++y) {
				dvec2 off(0);
				ll x = x_;
				if (x_ < 0) {
					x = x_ + BG_GRID_WIDTH;
					off = -space_max_duplicate_off_vec;
				} else if (x_ > (ll)BG_GRID_WIDTH-1) {
					x = x_ - BG_GRID_WIDTH;
					off = space_max_duplicate_off_vec;
				}
				assert(in_between_inclusive(0ll, (ll)BG_GRID_WIDTH-1, x));
				if (bg_grid[y][x] != INVALID_ID) {
					const dvec2 &q = joints[bg_grid[y][x]];
					if (len_sq(q+off-p) <= RR)
						distant = false;
				}
			}
		}
		if (distant) {
			active_list.push_back(joints.size());
			bg_grid[grid_p.y][grid_p.x] = joints.size();
			joints.push_back(p);
		}
		return distant;
	};

	add_point({
		std::uniform_real_distribution<double>(0.0, space_max.x)(gen),
		std::uniform_real_distribution<double>(0.0, space_max.y)(gen),
	});

	while (not active_list.empty()) {
		const std::size_t id_list = std::uniform_int_distribution<std::size_t>(
			0, active_list.size()-1)(gen);
		const dvec2 &p = joints[active_list[id_list]];

		constexpr std::size_t MAX_ITERATIONS = 30;
		std::size_t i = 0;
		for (; i < MAX_ITERATIONS; ++i) {
			const double alpha = alpha_distrib(gen);
			const double r = r_distrib(gen);
			const dvec2 q = dvec2(
				std::cos(alpha) * r,
				std::sin(alpha) * r
			) + p;
			if (add_point(q))
				break;
		}
		if (i == MAX_ITERATIONS) {
			active_list[id_list] = active_list.back();
			active_list.pop_back();
		}
	}

	// for (const dvec2 &p : joints) {
	// 	draw_point(map_storage, p, 0.001, 0x4f4f4f);
	// 	draw_point(map_storage, p + space_max_duplicate_off_vec, 0.001, 0x4f4f4f);
	// }
}

void map_generator_t::generate_rivers(std::mt19937 &gen) {
	const double R = global_settings.river_joints_R;
	const double RR = R*R;
	const std::size_t joints_cnt = joints.size();
	std::uniform_int_distribution<int> probability_distrib(1, 100);
	al.assign(joints_cnt, {  });
	joints_humidity.assign(joints_cnt, GREATEST_WATER_DIST);

	std::vector<double> joints_coords(joints_cnt * 2 * 3);
	for (std::size_t i = 0; i < joints_cnt; ++i) {
		joints_coords[2*i+0 + 0*joints_cnt] = joints[i].x
			+ 1.0*space_max.x;
		joints_coords[2*i+1 + 0*joints_cnt] = joints[i].y;

		joints_coords[2*i+0 + 2*joints_cnt] = joints[i].x;
		joints_coords[2*i+1 + 2*joints_cnt] = joints[i].y;

		joints_coords[2*i+0 + 4*joints_cnt] = joints[i].x
			+ 2.0*space_max.x;
		joints_coords[2*i+1 + 4*joints_cnt] = joints[i].y;
	}
	const delaunator::Delaunator d(joints_coords);

	for (std::size_t half_edge = 0;
			half_edge < d.triangles.size();
			++half_edge) {
		assert(d.triangles[half_edge] < 3*joints_cnt);
		const std::size_t next_half_edge
			= (half_edge % 3 == 2) ? half_edge - 2 : half_edge + 1;

		std::size_t p = d.triangles[half_edge];
		std::size_t q = d.triangles[next_half_edge];

		// Omit extreme edges
		if (d.halfedges[half_edge] == delaunator::INVALID_INDEX)
			continue;

		// Omit duplicated `p` points
		if (p >= joints_cnt)
			continue;

		// Omit duplicated edges
		if (p > q && d.halfedges[half_edge] != delaunator::INVALID_INDEX)
			continue;

		// Omit very long edges
		{
			dvec2 v {
				joints_coords[2*p+0],
				joints_coords[2*p+1]
			};
			v -= dvec2(
				joints_coords[2*q+0],
				joints_coords[2*q+1]
			);
			if (len_sq(v) > 3*3*RR)
				continue;
		}

		joint_edge_t e1, e2;
		if (q >= joints_cnt) {
			q -= joints_cnt;
			if (q >= joints_cnt) {
				q -= joints_cnt;
				e1 = {q, joint_edge_t::TO_RIGHT};
				e2 = {p, joint_edge_t::TO_LEFT};
			} else {
				e1 = {q, joint_edge_t::TO_LEFT};
				e2 = {p, joint_edge_t::TO_RIGHT};
			}
		} else {
			e1 = {q, joint_edge_t::USUAL};
			e2 = {p, joint_edge_t::USUAL};
		}
		al[p].push_back(e1);
		al[q].push_back(e2);
	}

	const int river_start_prob = global_settings.river_start_prob;
	const int river_branch_prob = global_settings.river_branch_prob;
	std::queue<std::size_t> next_v;
	std::vector<std::size_t> parent(joints_cnt, INVALID_ID);
	std::vector<dvec2> parent_edge(joints_cnt);
	for (std::size_t i = 0; i < joints_cnt; ++i) {
		const dvec2 &p = joints[i] + space_max_duplicate_off_vec;
		const double A = get_elevation_A(p);
		if (A >= 0.5) continue;
		parent[i] = i;
		for (joint_edge_t &e : al[i]) {
			if (parent[e.dest] != INVALID_ID) continue;
			dvec2 q = joints[e.dest] + space_max_duplicate_off_vec;
			const double B = get_elevation_A(q);
			if (B < 0.5) continue;
			if (not (probability_distrib(gen) <= river_start_prob)) continue;

			if (e.type == joint_edge_t::TO_LEFT)
				q -= space_max_duplicate_off_vec;
			else if (e.type == joint_edge_t::TO_RIGHT)
				q += space_max_duplicate_off_vec;
			parent[e.dest] = i;
			parent_edge[e.dest] = p-q;
			e.river = true;
			joints_humidity[i] = 0;
			joints_humidity[e.dest] = 0;
			next_v.push(e.dest);
		}
	}

	while (not next_v.empty()) {
		const std::size_t v = next_v.front();
		next_v.pop();
		const dvec2 &p = joints[v] + space_max_duplicate_off_vec;
		const double A = get_elevation_A(p);
		bool first_child = true;
		for (joint_edge_t &e : al[v]) {
			if (parent[e.dest] != INVALID_ID) continue;
			dvec2 q = joints[e.dest] + space_max_duplicate_off_vec;
			const double B = get_elevation_A(q);
			if (not (A < B + 0.03) || B < 0.5) continue;
			if ( not first_child &&
				not (probability_distrib(gen) <= river_branch_prob)) continue;
			{
				if (e.type == joint_edge_t::TO_LEFT)
					q -= space_max_duplicate_off_vec;
				else if (e.type == joint_edge_t::TO_RIGHT)
					q += space_max_duplicate_off_vec;
				const dvec2 a = q-p;
				const dvec2 &b = parent_edge[v];
				const double prod = dot_product(a, b);
				const double cos_sq = prod*prod / len_sq(a) / len_sq(b);
				const double cs = std::sqrt(cos_sq) * sign_of(prod);
				const double alpha = std::acos(cs);
				if (alpha <= 3.0*M_PI_4) continue;
			}

			parent[e.dest] = v;
			parent_edge[e.dest] = p-q;
			first_child = false;
			e.river = true;
			joints_humidity[e.dest] = 0;
			next_v.push(e.dest);
		}
	}

	const uint32_t river_color = global_settings.river_color;
	for (std::size_t i = 0; i < joints_cnt; ++i) {
		const dvec2 A = space_max_duplicate_off_vec + joints[i];
		for (const joint_edge_t &e : al[i]) {
			if (not e.river) continue;
			const dvec2 B = space_max_duplicate_off_vec + joints[e.dest];
			if (e.type == joint_edge_t::USUAL)
				draw_edge(
					A,
					B,
					river_color),
				draw_edge(
					A - space_max_duplicate_off_vec,
					B - space_max_duplicate_off_vec,
					river_color),
				draw_edge(
					A + space_max_duplicate_off_vec,
					B + space_max_duplicate_off_vec,
					river_color);
			// else if (e.type == joint_edge_t::TO_LEFT)
			// 	draw_edge(map_storage,
			// 		A,
			// 		B - space_max_duplicate_off_vec,
			// 		river_color);
			else if (e.type == joint_edge_t::TO_RIGHT)
				draw_edge(
					A,
					B + space_max_duplicate_off_vec,
					river_color),
				draw_edge(
					A - space_max_duplicate_off_vec,
					B,
					river_color);
		}
	}

	// for (std::size_t i = 0; i < joints_cnt; ++i) {
	// 	const dvec2 A = space_max_duplicate_off_vec + joints[i];
	// 	draw_point(map_storage, A, 0.01, 0xe2a3b5);
	// 	draw_point(map_storage, A - space_max_duplicate_off_vec, 0.01, 0xe2a3b5);
	// 	draw_point(map_storage, A + space_max_duplicate_off_vec, 0.01, 0xe2a3b5);
	// }
}

double map_generator_t::get_temperature(const glm::dvec2 &p) const {
	double elevation = get_elevation_A(p);
	double temperature = std::abs(space_max.y/2.0 - p.y) / (space_max.y/2.0);
	temperature = 1.0 - temperature;
	temperature = 1.0 - std::pow(1.0 - temperature,
			global_settings.temperature_exp);

	temperature = 0.6 * temperature + 0.4;
	elevation = (clamp(elevation, 0.5, 1.0) - 0.5) * 2.0;
	temperature -= elevation * 0.4;

	temperature = clamp(temperature, 0.0, 1.0);
	return temperature;
}

void map_generator_t::calculate_climate() {
	const std::size_t joints_cnt = joints.size();
	const int humidity_scale = global_settings.humidity_scale;

	std::queue<std::size_t> next_v;
	for (std::size_t v = 0; v < joints_cnt; ++v) {
		const dvec2 p = joints[v] + space_max_duplicate_off_vec;
		const double elevation = get_elevation_A(p);
		if (elevation < 0.5 || joints_humidity[v] == 0) {
			joints_humidity[v] = 0;
			next_v.push(v);
		}
	}

	while (not next_v.empty()) {
		const std::size_t v = next_v.front();
		next_v.pop();
		for (const joint_edge_t &e : al[v]) {
			if (!min_replace(joints_humidity[e.dest], joints_humidity[v]+1))
				continue;
			next_v.push(e.dest);
		}
	}

	for (std::size_t v = 0; v < joints_cnt; ++v) {
		const dvec2 p = joints[v] + space_max_duplicate_off_vec;

		float humidity = (float)joints_humidity[v] / (float)humidity_scale;
		min_replace(humidity, 1.0f);
		const double temperature = get_temperature(p);

		double hue = 0;
		if (global_settings.draw_humidity)
			hue = humidity * 120.0 / 360.0 + 240.0 / 360.0;
		else if (global_settings.draw_temperature)
			hue = (1.0 - temperature) * 300.0 / 360.0;

		const uint32_t color = hsv_to_rgb(hue, 0.67f, 0.86f);
		draw_point(p, 0.03, color);
	}
}

double map_generator_t::get_elevation_A(const glm::dvec2 &p) const {
	auto grid_p = space_to_grid_coords(p);
	min_replace<long long>(grid_p.x, grid_width-1);
	min_replace<long long>(grid_p.y, grid_height-1);
	const std::vector<voro_id_t> &vec
		= grid[grid_p.y][grid_p.x];

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
		std::size_t prev = triangle_neighbor_edge_id;
		std::size_t nxt = triangle_neighbor_edge_id;
		{
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
		) {
			elevation
				= casted_point_dist
				/ (casted_point_dist + closest_voro_dist);
			if (elevation <= 1.0)
				pixel_type = plate_t::COAST;
		} else if (
			(prev_type != plate_t::NONE || nxt_type != plate_t::NONE)
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

	double noise_val = noise.octave2D_01_warped(
		(p.x-space_max.x) * noise_pos_mult,
		p.y * noise_pos_mult,
		8);

	{
		const double x = elevation;
		const double xx = x*x;
		const double xxx = xx*x;
		elevation = (-3.0*xxx + 4.0*xx + x) / 2.0;
	}
	double noised_elevation
		= -0.4 + elevation * 0.8
		+ std::pow(noise_val, 1.3) * (0.6 + elevation * 0.0);
	noised_elevation = max(noised_elevation, 0.0);

	return noised_elevation;
}

void map_generator_t::draw_map_cpu([[maybe_unused]] std::mt19937 &gen) {
// #define DRAW_GRID
#ifdef DRAW_GRID
	for (double x = 0; x <= space_max.x; x += grid_box_dim_f) {
		draw_edge(
				dvec2(x, 0),
				dvec2(x, space_max.y),
				0x424242);
		draw_edge(
				dvec2(x+space_max.x, 0),
				dvec2(x+space_max.x, space_max.y),
				0x424242);
		draw_edge(
				dvec2(x+space_max.x*2, 0),
				dvec2(x+space_max.x*2, space_max.y),
				0x424242);
	}
	for (double y = 0; y < space_max.y; y += grid_box_dim_f) {
		draw_edge(
				dvec2(0, y),
				dvec2(real_space_max.x, y),
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
				draw_edge(map_storage,
						A,
						B diagram.duplicate_off_vec,
						0xfcf485);
			} else if (edge.type == edge.TO_RIGHT) {
				draw_edge(map_storage,
						A,
						B + diagram.duplicate_off_vec,
						0xfcb985);
			} else {
				draw_edge(map_storage,
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
	std::chrono::high_resolution_clock clock;
	const auto timer_start = clock.now();
	#pragma omp parallel for schedule (dynamic, 8)
	for (int y = 0; y < height; ++y) {
		for (std::size_t x = 0; x < static_cast<std::size_t>(width) / 3; ++x) {
			const dvec2 p = map_to_space_coords(dvec2(x+width/3, y));

			double elevation_A = get_elevation_A(p);
			assert(in_between_inclusive(0.0, 1.0, elevation_A));

			uint32_t color;
			if (elevation_A < 0.5)
				elevation_A = 0.15 + 0.25 * elevation_A;
			else
				elevation_A = 0.0 + 1.0 * elevation_A;

			// color = lerp(0, 0xff, elevation_A);
			// color = color | (color << 8) | (color << 16);

			color = hsv_to_rgb(
				(1.0 - elevation_A) * 240.0 / 360.0, 0.6, 0.8);

			// if (debug_vals[1] % 2 == 1) {
			// 	// PRINT_F(noise_val);
			// 	color = lerp(0xff, 0, noise_val);
			// 	color = color | (color << 8) | (color << 16);
			// }

			map_storage->set_rgb_value(y, x+width*0/3, color);
			map_storage->set_rgb_value(y, x+width*1/3, color);
			map_storage->set_rgb_value(y, x+width*2/3, color);

			const uint8_t elevation_A_byte = elevation_A * 255.0;
			map_storage->get_component_reference(y, x+width*0/3, 3)
				= elevation_A_byte;
			map_storage->get_component_reference(y, x+width*1/3, 3)
				= elevation_A_byte;
			map_storage->get_component_reference(y, x+width*2/3, 3)
				= elevation_A_byte;
		}
	}
	const auto diff = clock.now() - timer_start;
	const auto render_time_ms
		= std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
	PRINT_LD(render_time_ms);
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
		// draw_convex_polygon(map_storage, voronoi.points, color);
// #define DRAW_NOISY
#ifndef DRAW_NOISY
		for (std::size_t j = 0; j < voronoi.points.size(); ++j) {
			// draw_edge(map_storage,
			// 		voronoi.points[j],
			// 		voronoi.center,
			// 		// color, true);
			// 		0x9c9e9d, false);

			draw_edge(
					voronoi.points[j],
					voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1],
					// color, true);
					override_color, false);

			draw_edge(
					voronoi.points[j]
					- diagram.duplicate_off_vec,
					voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1]
					- diagram.duplicate_off_vec,
					// color, true);
					// 0xa2f9d9, false);
					override_color, false);

			draw_edge(
					voronoi.points[j]
					+ diagram.duplicate_off_vec,
					voronoi.points[j+1 == voronoi.points.size() ? 0 : j+1]
					+ diagram.duplicate_off_vec,
					// color, true);
					// 0xa2eef9, false);
					override_color, false);
		}

		for (std::size_t j = 0; j < voronoi.al.size(); ++j) {
			// draw_edge(map_storage,
			// 		voronoi.center + voronoi.al[j].to_mid,
			// 		voronoi.center,
			// 		// color, true);
			// 		0xd6d6d6, false);
			draw_point(
					voronoi.center + voronoi.al[j].to_mid,
					0.01, 0xd6d6d6);
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

	if (global_settings.draw_mid_polygons)
		for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
			draw_voronoi(i);
		}

	// const std::size_t gy = debug_vals[0]/grid_width;
	// const std::size_t gx = debug_vals[0]%grid_width;
	// PRINT_ZU(gy);
	// PRINT_ZU(gx);
	// PRINT_ZU(grid_width);
	// for (const std::size_t i
	// 		: grid[gy][gx]) {
	// 	draw_voronoi(i);
	// }

	// draw_voronoi(debug_vals[1], 0xff0000);
#endif
#undef DRAW_POLYGONS

// #define FLOOD_FILL
#ifdef FLOOD_FILL
	for (std::size_t i = 0; i < diagram.voronois_cnt(); ++i) {
		const voronoi_t &voronoi = diagram.voronois[i];
		fill(voronoi.center, COLORS[
				static_cast<uint8_t>(plates[i].type)]);

		fill(voronoi.center + diagram.duplicate_off_vec, COLORS[
				static_cast<uint8_t>(plates[i].type)]);

		fill(voronoi.center - diagram.duplicate_off_vec, COLORS[
				static_cast<uint8_t>(plates[i].type)]);
	}
#endif
#undef FLOOD_FILL

// #define DRAW_SPACE_CYCLIC_BORDER
#ifdef DRAW_SPACE_CYCLIC_BORDER
	draw_edge(
			dvec2(space_max.x * 1.0, 0),
			dvec2(space_max.x * 1.0, space_max.y),
			0x7c7c7c);
	draw_edge(
			dvec2(space_max.x * 2.0, 0),
			dvec2(space_max.x * 2.0, space_max.y),
			0x7c7c7c);
#endif
#undef DRAW_SPACE_CYCLIC_BORDER
}

void map_generator_t::generate_map() {
    PRINT_NL;
	std::mt19937 gen(seed_voronoi);
	PRINT_LU(seed_voronoi);
    // printf("seed_voronoi = %lu\n", seed_voronoi);
	generate_continents(gen);

	if (not global_settings.generate_with_gpu) {
		map_storage->clear();
		generate_grid_intersections();
		// std::size_t avg_cnt = 0;
		// std::size_t max_cnt = 0;
		// for (size_t y = 0; y < grid_height; ++y) {
		// 	for (size_t x = 0; x < grid_width; ++x) {
		// 		avg_cnt += grid[y][x].size();
		// 		max_replace(max_cnt, grid[y][x].size());
		// 	}
		// }
		// avg_cnt /= grid_height*grid_width;
		// PRINT_ZU(avg_cnt);
		// PRINT_ZU(max_cnt);
		// // for (const auto e : grid[1][0])
		// // 	PRINT_ZU(e);
		draw_map_cpu(gen);
		if (global_settings.generate_rivers) {
			generate_joints(gen);
			generate_rivers(gen);
			if (global_settings.draw_temperature or
					global_settings.draw_humidity)
				calculate_climate();
		}
		if (global_settings.draw_player)
			draw_tour_path(gen);
		map_storage->load_from_cpu_to_gpu_memory();
	} else {
		draw_map_gpu();
	}
}
