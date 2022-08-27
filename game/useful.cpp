#include "useful.hpp"

#ifdef DEBUG
	bool enable_breakpoints = false;
#endif

int floor_div(int num, int den) {
	if ((num^den) > 0)
		return num/den;
	else {
		div_t res = div(num, den);
		return res.rem
			? res.quot-1 : res.quot;
	}
}

uint32_t hsv_to_rgb(float h, float s, float v) {
	float r, g, b;

	const int i = std::floor(h * 6.0f);
	const float f = h * 6.0f - i;
	const float p = v * (1.0f - s);
	const float q = v * (1.0f - f * s);
	const float t = v * (1.0f - (1.0f - f) * s);

	switch(i % 6){
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}

	glm::u32vec3 result {r * 255.0f, g * 255.0f, b * 255.0f};
	result &= 0xff;
	return
		(result.r << 0 ) |
		(result.g << 8 ) |
		(result.b << 16);
}
