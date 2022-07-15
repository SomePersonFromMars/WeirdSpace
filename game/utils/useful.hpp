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

// inline bool operator<(const glm::ivec2 &a, const glm::ivec2 &b) {
// 	return a.x < b.x || (a.x == b.x && a.y < b.y);
// }

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

#endif
