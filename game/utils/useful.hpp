#pragma once
#ifndef USEFUL_HPP
#define USEFUL_HPP

#include <glm/glm.hpp>

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

#endif
