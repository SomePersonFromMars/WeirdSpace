#pragma once
#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "bitmap.hpp"
#include "noise.hpp"
#include "voronoi.hpp"

#include <random>
#include <functional>

// Catmull–Rom spline
double spline(double t, const std::function<double(const long long)> &f);
double spline_gradient(
		double t, const std::function<double(const long long)> &f);

struct generator_C_t {
	// Public functions
	generator_C_t(bitmap_t * const bitmap);

	void init();
	void deinit();
	void new_seed();
	void load_settings();
	void calculate_constants();

	void generate_bitmap();

	std::pair<glm::dvec2, glm::dvec2> get_tour_path_points(const double off);

	std::size_t * const debug_vals = global_settings.debug_vals;

private:
	// Private structures
	struct plate_t {
		enum type_t : uint8_t {
			NONE = 0,
			LAND,
			WATER,
			COAST,
		} type = NONE;
		uint32_t debug_color = 0x0;
	};
	static constexpr uint32_t COLORS[] {
		0x0,
		0x3a9648, // LAND
		0x77c4dd, // WATER
		0xfcf04b, // COAST
	};
	struct joint_edge_t {
		std::size_t dest;
		enum type_t : uint8_t {
			USUAL = 0,
			TO_LEFT,
			TO_RIGHT
		} type;
		bool river = false;
	};

	// Private functions
	inline glm::dvec2 space_to_bitmap_coords(glm::dvec2 pos) const;
	inline glm::dvec2 bitmap_to_space_coords(glm::dvec2 pos) const;
	void draw_edge(glm::dvec2 beg, glm::dvec2 end,
			uint32_t color, bool draw_only_empty = false);
	void draw_point(glm::dvec2 pos, double dim,
			uint32_t color);
	// Fills whole consistent black space starting at origin
	void fill(glm::dvec2 origin,
			uint32_t fill_color);
	void draw_convex_polygon(
			const std::vector<glm::dvec2> _points,
			const uint32_t color);
	void draw_noisy_edge(
			std::mt19937 &gen,
			const std::size_t level,
			const double amplitude,
			const glm::dvec2 A,
			const glm::dvec2 B,
			const glm::dvec2 X,
			const glm::dvec2 Y,
			const uint32_t color);

	void generate_continents(std::mt19937 &gen);
	void generate_grid_intersections();
	void generate_joints(std::mt19937 &gen);
	void generate_rivers(std::mt19937 &gen);
	void calculate_climate();
	void draw_map_cpu(std::mt19937 &gen);
	void draw_map_gpu();
	void draw_tour_path(std::mt19937 &gen);

	double get_temperature(const glm::dvec2 &p) const;
	double get_elevation_A(const glm::dvec2 &p) const;

	inline glm::tvec2<long long, glm::highp> space_to_grid_coords(
		const glm::dvec2 &p) const;

	// Private data
	// Bitmap
	bitmap_t * const bitmap;
	// Constants
	std::size_t grid_box_dim_zu;

	const int &width, &height;
	double ratio_wh, ratio_hw;
	glm::dvec2 space_max; // Maximum double coordinates
	double space_max_x_duplicate_off;
	glm::dvec2 space_max_duplicate_off_vec;

	std::size_t grid_height;
	std::size_t grid_width;

	double grid_box_dim_f;
	static constexpr int GREATEST_WATER_DIST = std::numeric_limits<int>::max();
	double chunk_dim_f;
	double noise_pos_mult;

	// Small variables
	const std::function<double(const long long)> get_tour_path_point_x;
	const std::function<double(const long long)> get_tour_path_point_y;
	std::mt19937::result_type seed_voronoi;
	cyclic_noise_t noise;

	// Voronoi diagram
	std::size_t voro_cnt;
	std::size_t super_voro_cnt;
	// Contains intersections of grid boxes with voronoi polygons
	std::vector<std::vector<std::vector<voro_id_t>>> grid;
	voronoi_diagram_t diagram;
	std::vector<plate_t> plates;

	// Joints etc.
	std::vector<glm::dvec2> tour_path_points;
	std::vector<glm::dvec2> joints;
	std::vector<std::vector<joint_edge_t>> al;
	std::vector<int> joints_humidity;

	// OpenGL names
	GLuint program1;
	GLuint program2;
	GLuint t_prog1_uniform;
	GLuint t_prog2_uniform;
	GLuint space_max_uniform;
	GLuint fbo;
	GLuint continents_vao;
	GLuint continents_triangle_pos_buf;
	GLuint continents_elevation_buf;
	// GLuint continents_type_buf;

	// OpenGL buffers helpers
	std::vector<glm::vec2> continents_triangles_pos;
	std::vector<float> continents_elevation;
	// std::vector<uint8_t> continents_type;
};


inline glm::dvec2 generator_C_t::space_to_bitmap_coords(glm::dvec2 pos) const {
	return
		pos.x = pos.x*double(width-1),
		pos.x /= double(width) / double(height),
		pos.y = pos.y*double(height-1),

		pos;
}
inline glm::dvec2 generator_C_t::bitmap_to_space_coords(glm::dvec2 pos) const {
	return
		pos.x = pos.x * space_max.x / double(width-1),
		pos.y = pos.y * space_max.y / double(height-1),

		pos;
}

inline glm::tvec2<long long, glm::highp> generator_C_t::space_to_grid_coords(
	const glm::dvec2 &p) const {
	return glm::tvec2<long long, glm::highp>(
			std::floor(
				(p.x-space_max.x/3.0) / grid_box_dim_f),
			std::floor(p.y / grid_box_dim_f)
		);
}

#endif
