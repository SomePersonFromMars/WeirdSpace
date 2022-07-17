#pragma once
#ifndef USEFUL_HPP
#define USEFUL_HPP

#include <glm/glm.hpp>

#define DEBUG
#ifdef DEBUG
	#define PRINT_ZU(var) printf(#var " = %zu\n", var)
	#define PRINT_F(var) printf(#var " = %f\n", var)
	#define WHERE printf("%s:%d\n", __func__, __LINE__)
#else
	#define PRINT_ZU(var)
	#define PRINT_F(var)
	#define WHERE
#endif

constexpr double PI = 3.14159265358979323846;

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define AT3(x, y, z) \
	[static_cast<size_t>(x)] \
	[static_cast<size_t>(y)] \
	[static_cast<size_t>(z)]

#define AT3_M(x, y, z, Mx, My, Mz) \
	[static_cast<size_t>((x%Mx+Mx)%Mx)] \
	[static_cast<size_t>((y%My+My)%My)] \
	[static_cast<size_t>((z%Mz+Mz)%Mz)]

struct ivec2_cmp_t {
	inline bool operator()(const glm::ivec2 &a, const glm::ivec2 &b) const {
		return a.x < b.x || (a.x == b.x && a.y < b.y);
	}
};

inline glm::vec3 color_hex_to_vec3(uint32_t hex) {
	return glm::vec3(
		static_cast<double>( (hex & 0xff0000u) >> 16  ) / 255.0,
		static_cast<double>( (hex & 0x00ff00u) >> 8  ) / 255.0,
		static_cast<double>( (hex & 0x0000ffu) >> 0 ) / 255.0
	);
}

inline glm::vec3 vec2_to_vec3(const glm::vec2& vec) {
	return glm::vec3(vec.x, vec.y, 0.0f);
}

inline float len_sq(const glm::vec2& vec) {
	return vec.x*vec.x + vec.y*vec.y;
}

template<class T, class S>
T lerp(T a, T b, S x) {
	x = glm::clamp<S>(x, 0, 1);
	return a*(static_cast<S>(1)-x) + b*x;
}

#endif
