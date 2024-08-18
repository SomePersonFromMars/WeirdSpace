// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef NOISE_HPP
#define NOISE_HPP

#include <perlin_noise.hpp>

struct cyclic_noise_t {
	using seed_type = siv::PerlinNoise::seed_type;
	inline void reseed(seed_type seed);
	double border_beg = 9;
	double border_end = 10;

	double octave2D_01(double x, double y, int octaves,
		double persistence = 0.5) const;

	double octave2D_01_warped(double x, double y, int octaves,
		double persistence = 0.5) const;

	double octave2D_01_double_warped(double x, double y, int octaves,
		double persistence = 0.5) const;

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
