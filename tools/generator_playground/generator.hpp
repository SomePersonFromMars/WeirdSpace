#pragma once
#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "bitmap.hpp"
#include "noise.hpp"

#include <random>

struct generator_A_t {
	generator_A_t();

	void new_seed();
	void generate_bitmap(bitmap_t &bitmap, int resolution_div);

private:
	const int &width, height;

	const double noise_pos_mult = 1.0/double(CHUNK_DIM)*3.0;
	glm::u8vec3 get(glm::ivec2 ipos);
	cyclic_noise_t noise;
};

struct generator_B_t {
	generator_B_t();

	void new_seed();
	void generate_bitmap(bitmap_t &bitmap);

private:
	const int &width, height;
	const float ratio_wh, ratio_hw;

	std::mt19937::result_type seed_voronoi;
	noise_t coast_noise;

	void draw_edge(bitmap_t &bitmap, glm::vec2 beg01, glm::vec2 end01,
			uint32_t color);
	void draw_point(bitmap_t &bitmap, glm::vec2 pos, float dim,
			uint32_t color);

	// edge_color is a border color
	void fill(bitmap_t &bitmap, glm::vec2 origin,
			uint32_t edge_color, uint32_t fill_color);

	struct tile_t {
		uint32_t type = 0;
		int coast_dist = 0;
		int perturbtion = 0;
		double tmp;

		static constexpr uint32_t WATER_BIT = 1;
		static constexpr uint32_t COAST_BIT = 4;
		static constexpr uint32_t LAND_BIT = 2;

		static constexpr int COAST_DEPTH = 32;
	};

	struct fractal_grid_t {
		glm::ivec2 size;
		int voronoi_cnt;
		float land_probability;
		std::vector<std::vector<tile_t>> grid;
		void generate_grid(std::mt19937::result_type seed_voronoi,
				noise_t &noise);
	};

	std::array<fractal_grid_t, 1> grids; // Fractal grids

	// void draw_hexagon(bitmap_t &bitmap, glm::ivec2 grid_pos,
	// 		uint32_t edge_color, uint32_t fill_color);

	// glm::ivec2 get_hex_neighbor(glm::ivec2 v, int id);
	// std::vector<std::vector<uint32_t>> plates; // Hexagons' values

	// void generate_grid(glm::ivec2 size);

	// static constexpr float tri_edge = 0.005;
	// static constexpr float tri_h = 0.866025404f; // Ratio of the triangle's
	// 											// height to its edge
	// static constexpr float hex_points[] {
	// 	-1, 0,
	// 	-0.5, tri_h,
	// 	0.5, tri_h,
	// 	1, 0,
	// 	0.5, -tri_h,
	// 	-0.5, -tri_h,
	// };
};

#endif
