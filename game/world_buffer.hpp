#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <map>
#include "useful.hpp"
#include "chunk.hpp"

struct world_buffer_t {
	const int width = 2; // World width in chunks
	std::map<glm::ivec2, chunk_t, ivec2_cmp_t> chunks;

	inline block_type& get(glm::ivec3 pos);

	// `pos` - Right-bottom-front rectangle position
	// `dimensions` - POSITIVE rectangle dimensions in XY plane
	// Returns `true` if rect intersects with blocks
	// 	different than block_type::none, `false` otherwise
	bool collision_check_XY_rect(glm::vec3 pos, glm::vec2 dimensions);

private:
	block_type void_block = block_type::none;
};

inline block_type& world_buffer_t::get(glm::ivec3 pos) {
	return 0 <= pos.y && pos.y <= static_cast<int>(chunk_t::HEIGHT) ?
		chunks[glm::ivec2(
			floor_div(pos.x, static_cast<int>(chunk_t::WIDTH)) % width,
			floor_div(pos.z, static_cast<int>(chunk_t::DEPTH))
		)].content AT3_M(
				pos.x, pos.y, pos.z,
				chunk_t::WIDTH, chunk_t::HEIGHT, chunk_t::DEPTH
			)
		: void_block;
}

#endif
