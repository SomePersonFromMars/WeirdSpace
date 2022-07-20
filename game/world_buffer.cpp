#include "world_buffer.hpp"

bool world_buffer_t::collision_check_XY_rect(
		glm::vec3 pos, glm::vec2 dimensions) {
	const glm::vec2 pos_end(pos.x+dimensions.x, pos.y+dimensions.y);

	glm::vec2 last_d;
	auto calc_last_d_on_axis
		= [&dimensions, &pos_end, &last_d] (float glm::vec2::* paxis) {

		if (dimensions.*paxis == std::floor(dimensions.*paxis)) {
			last_d.*paxis = dimensions.*paxis;
			if (pos_end.*paxis == std::floor(pos_end.*paxis))
				last_d.*paxis -= 1;
		} else {
			last_d.*paxis = dimensions.*paxis;
			if (pos_end.*paxis == std::floor(pos_end.*paxis))
				last_d.*paxis = std::floor(last_d.*paxis);
		}
	};
	calc_last_d_on_axis(&glm::vec2::x);
	calc_last_d_on_axis(&glm::vec2::y);

	for (float dx = 0; dx <= last_d.x;) {
		for (float dy = 0; dy <= last_d.y;) {
			const glm::ivec3 wanted_block_pos(
					std::floor(pos.x+dx),
					std::floor(pos.y+dy),
					std::floor(pos.z)
				);
			if (get(wanted_block_pos) != block_type::none)
				return true;

			dy += 1;
			if (dy > dimensions.y && dy - dimensions.y < 1)
				dy = dimensions.y;
		}

		dx += 1;
		if (dx > dimensions.x && dx - dimensions.x < 1)
			dx = dimensions.x;
	}
	return false;
}
