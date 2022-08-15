#pragma once
#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "bitmap.hpp"

#include "noise.hpp"
#include <MyGAL/FortuneAlgorithm.h>

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

	mygal::Diagram<float> generate_diagram(size_t points_cnt);
	void draw_edge(bitmap_t &bitmap,
			mygal::Diagram<float>::HalfEdge half_edge);
	mygal::Diagram<float> diagram;
};

#endif
