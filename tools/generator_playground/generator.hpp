#pragma once
#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "bitmap.hpp"
#include "noise.hpp"

struct generator_t {
	generator_t();

	void new_seed();
	glm::u8vec3 get(glm::ivec2 ipos);
	void generate_bitmap(bitmap_t &bitmap, int resolution_div);

private:
	const int &width, height;

	// const double noise_pos_mult = 0.04;
	const double noise_pos_mult = 1.0/double(CHUNK_DIM)*3.0;

	// siv::PerlinNoise noise;
	cyclic_noise_t noise;
	cyclic_noise_t::seed_type seed;
};

#endif
