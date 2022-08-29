#pragma once
#ifndef NOISE_HPP
#define NOISE_HPP

#include <perlin_noise.hpp>
#include <cmath>

struct cyclic_noise_t {
	using seed_type = siv::PerlinNoise::seed_type;
	inline void reseed(seed_type seed);
	double border_beg = 9;
	double border_end = 10;

	double octave2D_01(double x, double y, int octaves,
		double persistence = 0.5);

private:
	siv::PerlinNoise base;
};

inline void cyclic_noise_t::reseed(cyclic_noise_t::seed_type seed) {
	base.reseed(seed);
}

struct noise_t {
	using seed_type = siv::PerlinNoise::seed_type;
	inline void reseed(seed_type seed);

	double octave2D_01(double x, double y, int octaves,
		double persistence = 0.5);

private:
	siv::PerlinNoise base;
};

inline void noise_t::reseed(noise_t::seed_type seed) {
	base.reseed(seed);
}

#endif
