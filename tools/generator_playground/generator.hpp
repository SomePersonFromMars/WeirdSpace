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

	std::size_t debug_val = 2;

protected:
	const int &width, height;
	const float ratio_wh, ratio_hw;
	const glm::vec2 space_max {ratio_wh, 1}; // Maximum float coordinates

	void draw_edge(bitmap_t &bitmap, glm::vec2 beg01, glm::vec2 end01,
			uint32_t color);
	void draw_ray(bitmap_t &bitmap, glm::vec2 beg01, glm::vec2 mid01,
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

	enum class inters_t : uint8_t {
		// Counterclockwise sides order
		// (Should be clockwise, but Delaunator
		// coordinate system conflicts with the OpenGL's)
		LEFT = 0,
		BOTTOM,
		RIGHT,
		TOP,
		SIDES_CNT,
		INSIDE,
		OUTSIDE,
	};
	struct reduced_edge_t {
		glm::dvec2 beg, end;
		inters_t beg_inters = inters_t::OUTSIDE;
		inters_t end_inters = inters_t::OUTSIDE;
	};
	struct voronoi_t {
		bool complete = false;
		std::vector<std::size_t> tri_half_edges;
		std::vector<reduced_edge_t> red_edges;
		std::size_t last_incoming_red_edge_id
			= std::numeric_limits<std::size_t>::max();
		std::vector<glm::dvec2> points;
		// Means that the voronoi is clipped
		bool edge_voronoi = false;
	};

	// line: parallel to vector v and passing through P
	// horizontal segment: (0, S.y), (S.x, S.y)
	static std::pair<glm::dvec2, bool> intersect_line_h_segment(
			glm::dvec2 P, glm::dvec2 v, glm::dvec2 S);
	// line: parallel to vector v and passing through P
	// vertical segment: (S.x, 0), (S.x, S.y)
	static std::pair<glm::dvec2, bool> intersect_line_v_segment(
			glm::dvec2 P, glm::dvec2 v, glm::dvec2 S);

	static std::pair<glm::dvec2, inters_t>
		find_closest_box_intersection_directed_edge(
			glm::dvec2 P, glm::dvec2 v, glm::dvec2 S);

	static std::pair<reduced_edge_t, bool> trim_edge(
			glm::dvec2 beg, glm::dvec2 end, glm::dvec2 S
			);

	static std::pair<reduced_edge_t, bool> trim_inf_edge(
			const glm::dvec2 beg,
			const glm::dvec2 direction_vec,
			const glm::dvec2 S
			);
};

#endif
