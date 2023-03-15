#include "noise.hpp"

#include <cmath>
#include "useful.hpp"

double cyclic_noise_t::octave2D_01(double x, double y, int octaves,
	double persistence) const {
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

double cyclic_noise_t::octave2D_01_warped(double x, double y, int octaves,
	double persistence) const {

	double qx = octave2D_01(x, y + 123.456, octaves, persistence);
	double qy = octave2D_01(x, y + 789.012, octaves, persistence);

	qx *= 2.0;
	qx -= 1.0;
	qy *= 2.0;
	qy -= 1.0;

	x = x + qx;
	y = y + qy;

	if (x > border_end) x -= border_end;

	return octave2D_01(x, y, octaves, persistence);
}

double cyclic_noise_t::octave2D_01_double_warped(double x, double y,
		int octaves, double persistence) const {

	double qx = octave2D_01_warped(x, y + 123.456, octaves, persistence);
	double qy = octave2D_01_warped(x, y + 789.012, octaves, persistence);

	qx *= 2.0;
	qx -= 1.0;
	qy *= 2.0;
	qy -= 1.0;

	x = x + qx;
	y = y + qy;

	if (x > border_end) x -= border_end;

	return octave2D_01(x, y, octaves, persistence);
}

double noise_t::octave2D_01(double x, double y, int octaves,
	double persistence) {
	const double a = base.octave2D_01(x, y, octaves, persistence);
	return a;
}
