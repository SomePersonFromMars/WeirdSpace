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

struct generator_t {
	generator_t();
	virtual void init() = 0;
	virtual void new_seed() = 0;
	virtual void generate_bitmap(bitmap_t &bitmap) = 0;
	virtual void load_settings() = 0;

	std::size_t * const debug_vals = global_settings.debug_vals;

protected:
	const int &width, &height;
public:
	const double ratio_wh, ratio_hw;
	const glm::dvec2 space_max {ratio_wh, 1}; // Maximum double coordinates
	const double space_max_x_duplicate_off = space_max.x * 1.0 / 3.0;
	const glm::dvec2 space_max_duplicate_off_vec {
		space_max_x_duplicate_off, 0};
protected:

	inline glm::dvec2 space_to_bitmap_coords(glm::dvec2 pos) const;
	inline glm::dvec2 bitmap_to_space_coords(glm::dvec2 pos) const;
	void draw_edge(bitmap_t &bitmap, glm::dvec2 beg, glm::dvec2 end,
			uint32_t color, bool draw_only_empty = false);
	void draw_point(bitmap_t &bitmap, glm::dvec2 pos, double dim,
			uint32_t color);
	// Fills whole consistent black space starting at origin
	void fill(bitmap_t &bitmap, glm::dvec2 origin,
			uint32_t fill_color);
	void draw_convex_polygon(bitmap_t &bitmap,
			const std::vector<glm::dvec2> _points,
			const uint32_t color);
	void draw_noisy_edge(bitmap_t &bitmap,
			std::mt19937 &gen,
			const std::size_t level,
			const double amplitude,
			const glm::dvec2 A,
			const glm::dvec2 B,
			const glm::dvec2 X,
			const glm::dvec2 Y,
			const uint32_t color);
};

struct generator_C_t : generator_t {
	// Public functions
	generator_C_t();

	virtual void init() override;
	virtual void new_seed() override;
	virtual void load_settings() override;

	virtual void generate_bitmap(bitmap_t &bitmap) override;

	std::pair<glm::dvec2, glm::dvec2> get_tour_path_points(const double off);

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
	void generate_continents(std::mt19937 &gen);
	void generate_grid_intersections();
	void generate_joints(std::mt19937 &gen, bitmap_t &bitmap);
	void generate_rivers(std::mt19937 &gen, bitmap_t &bitmap);
	void calculate_climate(bitmap_t &bitmap);
	void draw_map(bitmap_t &bitmap, std::mt19937 &gen);
	void draw_tour_path(bitmap_t &bitmap, std::mt19937 &gen);

	double get_temperature(const glm::dvec2 &p) const;
	double get_elevation_A(const glm::dvec2 &p) const;

	inline glm::tvec2<long long, glm::highp> space_to_grid_coords(
		const glm::dvec2 &p) const;

	// Private data
	// Constants
	static constexpr std::size_t GRID_BOX_DIM_ZU = CHUNK_DIM/4;
	const double GRID_BOX_DIM_F
		= static_cast<double>(GRID_BOX_DIM_ZU)
		* space_max.y / static_cast<double>(height);
	const std::size_t GRID_HEIGHT;
	const std::size_t GRID_WIDTH;
	static constexpr int GREATEST_WATER_DIST = std::numeric_limits<int>::max();
	const double CHUNK_DIM_F
		= static_cast<double>(CHUNK_DIM)
		* space_max.y / static_cast<double>(height);
	const double noise_pos_mult = 1.0/double(CHUNK_DIM_F)*2.0;

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
};


inline glm::dvec2 generator_t::space_to_bitmap_coords(glm::dvec2 pos) const {
	return
		pos.x = pos.x*double(width-1),
		pos.x /= double(width) / double(height),
		pos.y = pos.y*double(height-1),

		pos;
}
inline glm::dvec2 generator_t::bitmap_to_space_coords(glm::dvec2 pos) const {
	return
		pos.x = pos.x * space_max.x / double(width-1),
		pos.y = pos.y * space_max.y / double(height-1),

		pos;
}

inline glm::tvec2<long long, glm::highp> generator_C_t::space_to_grid_coords(
	const glm::dvec2 &p) const {
	return glm::tvec2<long long, glm::highp>(
			std::floor(
				(p.x-space_max.x/3.0) / GRID_BOX_DIM_F),
			std::floor(p.y / GRID_BOX_DIM_F)
		);
}

struct generator_D_t : generator_t {
	// Public functions
	generator_D_t();

	virtual void init() override;
	virtual void new_seed() override;
	virtual void load_settings() override;

	virtual void generate_bitmap(bitmap_t &bitmap) override;

private:
	GLuint program_id;
};

#endif
