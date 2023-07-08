#include "map_generator.hpp"

#include <queue>

#include <glm/glm.hpp>
using namespace glm;

#include <useful.hpp>
#include <geometry.hpp>

void map_generator_t::draw_edge(dvec2 beg, dvec2 end,
		uint32_t color, bool draw_only_empty) {
	beg = space_to_map_coords(beg);
	end = space_to_map_coords(end);

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
		if (!draw_only_empty || map_storage->get(pos.y, pos.x) == 0x0)
			map_storage->set(pos.y, pos.x, color);
	}
}

void map_generator_t::draw_point(glm::dvec2 pos, double dim,
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
			map_storage->set(y, x, color);
		}
	}
}

void map_generator_t::fill(glm::dvec2 origin,
		uint32_t fill_color) {
	origin = space_to_map_coords(origin);

	ivec2 first_pixel(origin.x, origin.y);
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
	map_storage->set(first_pixel.y, first_pixel.x, fill_color);

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

			const uint32_t cur_color = map_storage->get(p.y, p.x);
			if (cur_color != 0x000000)
				continue;

			next_pixel.push(p);
			map_storage->set(p.y, p.x, fill_color);
		}
	}
}

void map_generator_t::draw_convex_polygon(
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
		points[i] = space_to_map_coords(_points[i]);
		min_replace(min_x, points[i].x);
		max_replace(max_x, points[i].x);
		min_replace(min_y, points[i].y);
		max_replace(max_y, points[i].y);
		points[i] = space_to_map_coords(_points[i])
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
			if (inside && map_storage->get(y, x) == 0x0)
				map_storage->set(y, x, color);
		}
	}
}

// void map_generator_t::draw_convex_polygon(map_storage_t &map_storage,
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
// 		points[i] = space_to_map_coords(_points[i]);
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
// 				map_storage.set(y, x, color);
// 		}
// 	}
// }

void map_generator_t::draw_noisy_edge(
		std::mt19937 &gen,
		const std::size_t level,
		const double amplitude,
		const glm::dvec2 A,
		const glm::dvec2 B,
		const glm::dvec2 X,
		const glm::dvec2 Y,
		const uint32_t color) {
	if (level == 0) {
		draw_edge(A, B, color);
	} else {
		std::uniform_real_distribution<double> distrib(
				0.5-amplitude, 0.5+amplitude);
		const dvec2 P = lerp(X, Y, distrib(gen));
		draw_noisy_edge(
				gen,

				level-1,
				amplitude,

				A, P,
				(A+X)/2.0,
				(A+Y)/2.0,
				color
				);
		draw_noisy_edge(
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
