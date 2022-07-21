#include "world_buffer.hpp"

bool world_buffer_t::collision_check_XY_rect(
		glm::vec3 pos, glm::vec2 dimensions) {
	const glm::vec2 pos_end(pos.x+dimensions.x, pos.y+dimensions.y);

	glm::vec2 iterations_cnt(
			std::ceil(dimensions.x)+1,
			std::ceil(dimensions.y)+1
		);
	if (pos_end.x == std::floor(pos_end.x))
		iterations_cnt.x -= 1;
	if (pos_end.y == std::floor(pos_end.y))
		iterations_cnt.y -= 1;

	for (float dx = 0, j = 0; j < iterations_cnt.x; j += 1) {
		for (float dy = 0, i = 0; i < iterations_cnt.y; i += 1) {
			const glm::ivec3 wanted_block_pos(
					std::floor(pos.x+dx),
					std::floor(pos.y+dy),
					std::floor(pos.z)
				);
			if (get(wanted_block_pos) != block_type::none)
				return true;

			dy += 1;
			// if (dy > dimensions.y && dy - dimensions.y < 1)
			if (dy > dimensions.y)
				dy = dimensions.y;
		}

		dx += 1;
		// if (dx > dimensions.x && dx - dimensions.x < 1)
		if (dx > dimensions.x)
			dx = dimensions.x;
	}
	return false;
}
