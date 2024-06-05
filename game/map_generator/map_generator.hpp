#pragma once
#ifndef MAP_GENERATOR_HPP
#define MAP_GENERATOR_HPP

#include "map_storage.hpp"
#include "noise.hpp"
#include "voronoi.hpp"

#include <random>
#include <functional>

// Catmull–Rom spline
double spline(double t, const std::function<double(const long long)> &f);
double spline_gradient(
		double t, const std::function<double(const long long)> &f);

struct map_generator_t {
	// Basic usage functions
	map_generator_t(map_storage_t * const map_storage);
	void load_settings();
	void init_gl();
	void new_seed();
	void generate_map();
	void deinit_gl();

	// State query functions
	inline glm::dvec2 get_space_max() const;
	inline double get_ratio_wh() const;
	inline double get_ratio_hw() const;
	inline bool are_tour_path_points_generated() const;
	std::pair<glm::dvec2, glm::dvec2> get_tour_path_points(const double off);
    inline std::mt19937::result_type get_current_voronoi_seed() const;

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
	void calculate_constants();
	void set_uniforms();

	inline glm::dvec2 space_to_map_coords(glm::dvec2 pos) const;
	inline glm::dvec2 map_to_space_coords(glm::dvec2 pos) const;
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
	// Map storage
	map_storage_t * const map_storage;
	// Constants
	std::size_t grid_box_dim_zu;

	// Dimensions of the map storage
	const int &width, &height;
	// Ratios of the map storage coordinates
	double ratio_wh, ratio_hw;
	glm::dvec2 space_max; // Maximum double coordinates of not tripled space
	glm::dvec2 real_space_max; // Maximum double coordinates of tripled space
	double space_max_x_duplicate_off;
	glm::dvec2 space_max_duplicate_off_vec;

	std::size_t grid_height;
	std::size_t grid_width;

	double grid_box_dim_f;
	static constexpr int GREATEST_WATER_DIST = std::numeric_limits<int>::max();
	double noise_pos_mult;

	// Small variables
	const std::function<double(const long long)> get_tour_path_point_x;
	const std::function<double(const long long)> get_tour_path_point_y;
	std::mt19937::result_type seed_voronoi;
	cyclic_noise_t noise;
    long app_start_ms = -1;

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
#define PROG 2
	GLuint program1;
	GLuint program2;
	GLuint t_prog1_uniform;
	GLuint t_prog2_uniform;
	GLuint space_max_uniform;
	GLuint triple_map_size_uniform;
	GLuint fbo;
	GLuint continents_vao;
	GLuint continents_triangle_pos_buf;
	GLuint continents_elevation_buf;

	// OpenGL buffers helpers
	std::vector<glm::vec2> continents_triangles_pos;
	std::vector<float> continents_elevation;
};

inline glm::dvec2 map_generator_t::get_space_max() const {
	return space_max;
}

inline double map_generator_t::get_ratio_wh() const {
	return ratio_wh;
}

inline double map_generator_t::get_ratio_hw() const {
	return ratio_hw;
}

inline bool map_generator_t::are_tour_path_points_generated() const {
	return tour_path_points.size() > 0;
}

inline std::mt19937::result_type map_generator_t::get_current_voronoi_seed() const {
    return seed_voronoi;
}

inline glm::dvec2 map_generator_t::space_to_map_coords(glm::dvec2 pos) const {
	return
		pos.x = pos.x * double(width-1) / (3.0*space_max.x),
		pos.y = pos.y * double(height-1) / space_max.y,

		pos;
}
inline glm::dvec2 map_generator_t::map_to_space_coords(glm::dvec2 pos) const {
	return
		pos.x = pos.x * (3.0*space_max.x) / double(width-1),
		pos.y = pos.y * space_max.y / double(height-1),

		pos;
}

inline glm::tvec2<long long, glm::highp> map_generator_t::space_to_grid_coords(
	const glm::dvec2 &p) const {
	return glm::tvec2<long long, glm::highp>(
			std::floor(
				(p.x-space_max.x/3.0) / grid_box_dim_f),
			std::floor(p.y / grid_box_dim_f)
		);
}

#endif
