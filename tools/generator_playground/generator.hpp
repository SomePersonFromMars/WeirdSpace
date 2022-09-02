#pragma once
#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "bitmap.hpp"
#include "noise.hpp"

#include <random>

struct generator_t {
	generator_t();
	virtual void new_seed() = 0;
	virtual void generate_bitmap(bitmap_t &bitmap, int resolution_div) = 0;

protected:
	const int &width, height;
	const float ratio_wh, ratio_hw;

	void draw_edge(bitmap_t &bitmap, glm::vec2 beg01, glm::vec2 end01,
			uint32_t color);
	void draw_point(bitmap_t &bitmap, glm::vec2 pos, float dim,
			uint32_t color);
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

	void draw_edge(bitmap_t &bitmap, glm::vec2 beg01, glm::vec2 end01,
			uint32_t color);
	void draw_point(bitmap_t &bitmap, glm::vec2 pos, float dim,
			uint32_t color);

	// edge_color is a border color
	void fill(bitmap_t &bitmap, glm::vec2 origin,
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
		int voronoi_cnt;
		float land_probability;
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

private:
	std::mt19937::result_type seed_voronoi;

	glm::vec2 triangle_circumcenter(
			glm::vec2 A, glm::vec2 B, glm::vec2 C) const;
};

#endif
