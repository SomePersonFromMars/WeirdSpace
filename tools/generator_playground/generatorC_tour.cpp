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

double spline(double t, const std::function<double(const long long)> &f) {
	const double floor_t = std::floor(t);

	const long long p1 = static_cast<long long>(floor_t) + 0;
	const long long p2 = p1 + 1;
	const long long p3 = p2 + 1;
	const long long p0 = p1 - 1;

	t -= floor_t;
	const double tt = t * t;
	const double ttt = tt * t;

	const double q1 = -ttt + 2.0f*tt - t;
	const double q2 = 3.0f*ttt - 5.0f*tt + 2.0f;
	const double q3 = -3.0f*ttt + 4.0f*tt + t;
	const double q4 = ttt - tt;

	const double res
		= 0.5 * (
			f(p0) * q1 + f(p1) * q2 + f(p2) * q3 + f(p3) * q4
			);

	return res;
}

double spline_gradient(
		double t, const std::function<double(const long long)> &f) {
	const double floor_t = std::floor(t);

	const long long p1 = static_cast<long long>(floor_t) + 0;
	const long long p2 = p1 + 1;
	const long long p3 = p2 + 1;
	const long long p0 = p1 - 1;

	t -= floor_t;
	const double tt = t * t;

	const double q1 = -3.0 * tt + 4.0*t - 1;
	const double q2 = 9.0*tt - 10.0*t;
	const double q3 = -9.0*tt + 8.0*t + 1.0;
	const double q4 = 3.0*tt - 2.0*t;

	const double res
		= 0.5 * (
			f(p0) * q1 + f(p1) * q2 + f(p2) * q3 + f(p3) * q4
			);

	return res;
}

std::pair<glm::dvec2, glm::dvec2>
generator_C_t::get_tour_path_points(const double off) {
	const dvec2 pos(
			spline(off, get_tour_path_point_x),
			spline(off, get_tour_path_point_y)
			);
	const dvec2 gradient(
			spline_gradient(off, get_tour_path_point_x),
			spline_gradient(off, get_tour_path_point_y)
			);

	return { pos, gradient };
}

void generator_C_t::draw_tour_path(bitmap_t &bitmap, std::mt19937 &gen) {
	const double duplicate_off_x = diagram.space_max_x_duplicate_off;

	std::size_t land_voronois_cnt = 0;
	std::size_t water_voronois_cnt = 0;
	for (std::size_t i = 0; i < plates.size(); ++i) {
		if (plates[i].type == plate_t::LAND)
			++land_voronois_cnt;
		if (plates[i].type == plate_t::WATER)
			++water_voronois_cnt;
	}
	const std::size_t land_path_points_cnt = land_voronois_cnt * 80 / 100;
	const std::size_t water_path_points_cnt = std::min(
			land_path_points_cnt * 0 / 75,
			water_voronois_cnt
			);
	const std::size_t path_points_cnt
		= land_path_points_cnt + water_path_points_cnt;

	std::vector<double> chosen_points_coords(path_points_cnt*2 *2);
	{
		std::size_t j = 0;
		for (std::size_t i = 0; j < land_path_points_cnt; ++i) {
			if (plates[i].type != plate_t::LAND)
				continue;
			assert(i < diagram.voronois_cnt());
			chosen_points_coords[2*j+0]
				= diagram.voronois[i].center.x;
			chosen_points_coords[2*j+1]
				= diagram.voronois[i].center.y;
			chosen_points_coords[2*j+0 + 2*path_points_cnt]
				= diagram.voronois[i].center.x
				- duplicate_off_x;
			chosen_points_coords[2*j+1 + 2*path_points_cnt]
				= diagram.voronois[i].center.y;
			++j;
		}

		for (std::size_t i = 0; j < path_points_cnt; ++i) {
			if (plates[i].type != plate_t::WATER)
				continue;
			assert(i < diagram.voronois_cnt());
			chosen_points_coords[2*j+0]
				= diagram.voronois[i].center.x;
			chosen_points_coords[2*j+1]
				= diagram.voronois[i].center.y;
			chosen_points_coords[2*j+0 + 2*path_points_cnt]
				= diagram.voronois[i].center.x
				- duplicate_off_x;
			chosen_points_coords[2*j+1 + 2*path_points_cnt]
				= diagram.voronois[i].center.y;
			++j;
		}
	}

	// for (std::size_t i = 0; i < chosen_points_coords.size() / 2; i += 2) {
	// 	draw_point(bitmap,
	// 			dvec2(chosen_points_coords[i], chosen_points_coords[i+1]),
	// 			0.01, 0xef6b81);

	// 	draw_point(bitmap,
	// 			dvec2(
	// 				chosen_points_coords[i]
	// 				- duplicate_off_x,
	// 				chosen_points_coords[i+1]),
	// 			0.01, 0xef6b81);

	// 	draw_point(bitmap,
	// 			dvec2(
	// 				chosen_points_coords[i]
	// 				+ duplicate_off_x,
	// 				chosen_points_coords[i+1]),
	// 			0.01, 0xef6b81);
	// }

	const delaunator::Delaunator d(chosen_points_coords);

	struct weighted_edge_t {
		std::size_t a, b;
		double len_sq;
	};
	std::vector<weighted_edge_t> all_edges;
	weighted_edge_t cyclic_edge;
	cyclic_edge.a = INVALID_ID;
	cyclic_edge.b = INVALID_ID;
	cyclic_edge.len_sq = std::numeric_limits<double>::infinity();
	std::vector<std::vector<std::size_t>> al(path_points_cnt);

	for (std::size_t half_edge = 0;
			half_edge < d.triangles.size();
			++half_edge) {
		const std::size_t next_half_edge
			= (half_edge % 3 == 2) ? half_edge - 2 : half_edge + 1;

		weighted_edge_t weighted_edge;
		std::size_t p = weighted_edge.a = d.triangles[half_edge];
		std::size_t q = weighted_edge.b = d.triangles[next_half_edge];

		// Omit duplicated edges
		if (p > q && d.halfedges[half_edge] != delaunator::INVALID_INDEX)
			continue;

		weighted_edge.len_sq
			= len_sq(
			dvec2(chosen_points_coords[2*q+0], chosen_points_coords[2*q+1]) -
			dvec2(chosen_points_coords[2*p+0], chosen_points_coords[2*p+1])
			);

		if (p < path_points_cnt && q < path_points_cnt)
			all_edges.push_back(weighted_edge);
		else {
			if (p > q)
				std::swap(p, q);
			if (
				p < path_points_cnt && 	// One of this two points
										// isn't a duplicate
				weighted_edge.len_sq < cyclic_edge.len_sq
			) {
				cyclic_edge.len_sq = weighted_edge.len_sq;
				cyclic_edge.a = p;
				cyclic_edge.b = q - path_points_cnt;
			}
		}
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
		al[e.b].push_back(e.a);
	}

	// for (std::size_t v = 0; v < path_points_cnt; ++v) {
	// 	for (const std::size_t w : al[v]) {
	// 		draw_edge(bitmap,
	// 			dvec2(
	// 				chosen_points_coords[2*v+0],
	// 				chosen_points_coords[2*v+1]),
	// 			dvec2(
	// 				chosen_points_coords[2*w+0],
	// 				chosen_points_coords[2*w+1]),
	// 			0x9e0922
	// 			);

	// 		draw_edge(bitmap,
	// 			dvec2(
	// 				chosen_points_coords[2*v+0]
	// 				- duplicate_off_x,
	// 				chosen_points_coords[2*v+1]),
	// 			dvec2(
	// 				chosen_points_coords[2*w+0]
	// 				- duplicate_off_x,
	// 				chosen_points_coords[2*w+1]),
	// 			0x9e0922
	// 			);

	// 		draw_edge(bitmap,
	// 			dvec2(
	// 				chosen_points_coords[2*v+0]
	// 				+ duplicate_off_x,
	// 				chosen_points_coords[2*v+1]),
	// 			dvec2(
	// 				chosen_points_coords[2*w+0]
	// 				+ duplicate_off_x,
	// 				chosen_points_coords[2*w+1]),
	// 			0x9e0922
	// 			);
	// 	}
	// }

	assert(cyclic_edge.len_sq != std::numeric_limits<double>::infinity());
	// draw_edge(bitmap,
	// 	dvec2(
	// 		chosen_points_coords[2*cyclic_edge.a+0],
	// 		chosen_points_coords[2*cyclic_edge.a+1]),
	// 	dvec2(
	// 		chosen_points_coords[2*cyclic_edge.b+0]
	// 		- duplicate_off_x,
	// 		chosen_points_coords[2*cyclic_edge.b+1]),
	// 	0x9e0922
	// 	);
	// draw_edge(bitmap,
	// 	dvec2(
	// 		chosen_points_coords[2*cyclic_edge.a+0]
	// 		+ duplicate_off_x,
	// 		chosen_points_coords[2*cyclic_edge.a+1]),
	// 	dvec2(
	// 		chosen_points_coords[2*cyclic_edge.b+0],
	// 		chosen_points_coords[2*cyclic_edge.b+1]),
	// 	0x9e0922
	// 	);

	std::vector<std::size_t> cycle;

	std::vector<std::size_t> next_event;
	next_event.push_back(cyclic_edge.a);

	while (!next_event.empty()) {
		const std::size_t v = next_event.back();
		if (v == cyclic_edge.b)
			break;
		next_event.pop_back();
		if (v == INVALID_ID) {
			cycle.pop_back();
			continue;
		}

		next_event.push_back(INVALID_ID);
		for (const std::size_t w : al[v]) {
			if (!cycle.empty() && cycle.back() == w)
				continue;
			next_event.push_back(w);
		}

		cycle.push_back(v);
	}
	cycle.push_back(cyclic_edge.b);

	tour_path_points.resize(cycle.size());
	for (std::size_t i = 0; i < cycle.size(); ++i) {
		tour_path_points[i].x = chosen_points_coords[2*cycle[i]+0];
		tour_path_points[i].y = chosen_points_coords[2*cycle[i]+1];
	}

	for (
		double t = -static_cast<double>(cycle.size());
		t <= static_cast<double>(2*cycle.size()-1);
		t += 0.01)
	{
		draw_point(
			bitmap,
			dvec2(
				spline(t, get_tour_path_point_x),
				spline(t, get_tour_path_point_y)),
			0.001,
			0xcc99a1
			);
	}

	// for (std::size_t i = 0; ; ++i) {
	// 	draw_point(
	// 		bitmap,
	// 		dvec2(
	// 			get_tour_path_point_x(static_cast<long long>(i)
	// 				+ static_cast<long long>(cycle.size())),
	// 			get_tour_path_point_y(static_cast<long long>(i)
	// 				+ static_cast<long long>(cycle.size()))),
	// 		0.01,
	// 		0xef6b81
	// 		);

	// 	draw_point(
	// 		bitmap,
	// 		dvec2(
	// 			get_tour_path_point_x(static_cast<long long>(i)
	// 				- static_cast<long long>(cycle.size())),
	// 			get_tour_path_point_y(static_cast<long long>(i)
	// 				- static_cast<long long>(cycle.size()))),
	// 		0.01,
	// 		0xef6b81
	// 		);

	// 	draw_point(
	// 		bitmap,
	// 		dvec2(
	// 			get_tour_path_point_x(static_cast<long long>(i)),
	// 			get_tour_path_point_y(static_cast<long long>(i))),
	// 		0.01,
	// 		0xef6b81
	// 		);

	// 	if (i+1 == cycle.size())
	// 		break;

	// 	// draw_edge(bitmap,
	// 	// 	dvec2(
	// 	// 		chosen_points_coords[2*cycle[i+0]+0],
	// 	// 		chosen_points_coords[2*cycle[i+0]+1]),
	// 	// 	dvec2(
	// 	// 		chosen_points_coords[2*cycle[i+1]+0],
	// 	// 		chosen_points_coords[2*cycle[i+1]+1]),
	// 	// 	0x9e0922
	// 	// 	);

	// 	// draw_edge(bitmap,
	// 	// 	dvec2(
	// 	// 		chosen_points_coords[2*cycle[i+0]+0]
	// 	// 		- duplicate_off_x,
	// 	// 		chosen_points_coords[2*cycle[i+0]+1]),
	// 	// 	dvec2(
	// 	// 		chosen_points_coords[2*cycle[i+1]+0]
	// 	// 		- duplicate_off_x,
	// 	// 		chosen_points_coords[2*cycle[i+1]+1]),
	// 	// 	0x9e0922
	// 	// 	);

	// 	// draw_edge(bitmap,
	// 	// 	dvec2(
	// 	// 		chosen_points_coords[2*cycle[i+0]+0]
	// 	// 		+ duplicate_off_x,
	// 	// 		chosen_points_coords[2*cycle[i+0]+1]),
	// 	// 	dvec2(
	// 	// 		chosen_points_coords[2*cycle[i+1]+0]
	// 	// 		+ duplicate_off_x,
	// 	// 		chosen_points_coords[2*cycle[i+1]+1]),
	// 	// 	0x9e0922
	// 	// 	);
	// }
}
