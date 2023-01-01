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
	virtual void new_seed() = 0;
	virtual void generate_bitmap(bitmap_t &bitmap, int resolution_div) = 0;

	std::size_t debug_vals[2] = {
		7,
		0
	};
	// 84
	// 98
	// 109
	// 116
	// 122
	// 84
	// 16
	// 3
	// 109
	// 11
	// 72

protected:
	const int &width, &height;
public:
	const double ratio_wh, ratio_hw;
	const glm::dvec2 space_max {ratio_wh, 1}; // Maximum double coordinates
protected:

	inline glm::dvec2 space_to_bitmap_coords(glm::dvec2 pos);
	inline glm::dvec2 bitmap_to_space_coords(glm::dvec2 pos);
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

struct generator_A_t : generator_t {
	generator_A_t();

	virtual void new_seed() override;
	virtual void generate_bitmap(bitmap_t &bitmap,
			int resolution_div) override;

private:

	const double noise_pos_mult = 1.0/double(CHUNK_DIM)*3.0;
	glm::u8vec3 get(glm::ivec2 ipos);
	cyclic_noise_t noise;
};

struct generator_B_t : generator_t {
	generator_B_t();

	virtual void new_seed() override;
	virtual void generate_bitmap(bitmap_t &bitmap,
			int resolution_div) override;

private:
	std::mt19937::result_type seed_voronoi;
	noise_t coast_noise;

	// edge_color is a border color
	void fill(bitmap_t &bitmap, glm::dvec2 origin,
			uint32_t edge_color, uint32_t fill_color);

	struct tile_t {
		uint32_t type = 0;
		// int coast_dist = 0;
		int perturbtion = 0;
		glm::ivec2 coast_origin;
		double tmp;

		static constexpr uint32_t WATER_BIT = 1;
		static constexpr uint32_t COAST_BIT = 4;
		static constexpr uint32_t LAND_BIT = 2;

		static constexpr int COAST_DEPTH = 16;
		static constexpr long long COAST_DEPTH_SQ =
			static_cast<long long>(COAST_DEPTH) *
			static_cast<long long>(COAST_DEPTH);
	};

	struct fractal_grid_t {
		glm::ivec2 size;
		int voronois_cnt;
		double land_probability;
		std::vector<std::vector<tile_t>> grid;
		void generate_grid(std::mt19937::result_type seed_voronoi,
				noise_t &noise);
	};

	std::array<fractal_grid_t, 1> grids; // Fractal grids
};

struct generator_C_t : generator_t {
	generator_C_t();

	virtual void new_seed() override;
	virtual void generate_bitmap(bitmap_t &bitmap,
			int resolution_div) override;

	std::pair<glm::dvec2, glm::dvec2> get_tour_path_points(const double off);

private:
	void generate_continents(std::mt19937 &gen);
	void generate_grid_intersections();
	void draw_map(bitmap_t &bitmap, std::mt19937 &gen);
	void draw_tour_path(bitmap_t &bitmap, std::mt19937 &gen);

	const std::function<double(const long long)> get_tour_path_point_x;
	const std::function<double(const long long)> get_tour_path_point_y;

	std::mt19937::result_type seed_voronoi;

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

	static constexpr std::size_t voro_cnt = 240;
	static constexpr std::size_t super_voro_cnt = std::min<std::size_t>(
			40, voro_cnt);
	static constexpr double GRID_BOX_DIM_ZU = CHUNK_DIM/4;
	const double GRID_BOX_DIM_D
		= static_cast<double>(GRID_BOX_DIM_ZU)
		* space_max.y / static_cast<double>(height);
	const std::size_t GRID_HEIGHT;
	const std::size_t GRID_WIDTH;
	// Contains intersections of grid boxes with voronoi polygons
	std::vector<std::vector<std::vector<voro_id_t>>> grid;
	voronoi_diagram_t diagram;
	std::vector<plate_t> plates;
	std::vector<glm::dvec2> tour_path_points;
};

inline glm::dvec2 generator_t::space_to_bitmap_coords(glm::dvec2 pos) {
	return
		pos.x = pos.x*double(width-1),
		pos.x /= double(width) / double(height),
		pos.y = pos.y*double(height-1),

		pos;
}
inline glm::dvec2 generator_t::bitmap_to_space_coords(glm::dvec2 pos) {
	return
		pos.x = pos.x * space_max.x / double(width-1),
		pos.y = pos.y * space_max.y / double(height-1),

		pos;
}

#endif
