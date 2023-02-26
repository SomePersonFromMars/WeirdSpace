#include "noise.hpp"

#include <cmath>
#include "useful.hpp"

double cyclic_noise_t::octave2D_01(double x, double y, int octaves,
	double persistence) {
	double a = base.octave2D_01(x, y, octaves, persistence);
	if (x >= border_beg) {
		const double off = x - border_beg;
		const double border_len = border_end - border_beg;
		const double b = base.octave2D_01(x - border_end, y,
				octaves, persistence);
		const double t = off/border_len;
		a = lerp(a, b, t);
	}
	return a;
}

double noise_t::octave2D_01(double x, double y, int octaves,
	double persistence) {
	const double a = base.octave2D_01(x, y, octaves, persistence);
	return a;
}
