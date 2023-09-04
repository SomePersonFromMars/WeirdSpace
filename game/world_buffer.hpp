#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <vector>
#include <map>
#include <functional>

#include <useful.hpp>
#include <expiration_queue.hpp>
#include "chunk.hpp"

struct world_buffer_t {
	// World dimensions in chunks
	const int width = 6;
	const int height = 1;
	const int depth = 6;

	std::map<glm::ivec2, chunk_t, vec2_cmp_t<int>> chunks;
	inline block_type& get(glm::ivec3 pos);

	inline void for_each_active_chunk(const std::function<void(chunk_t&)> f);

	// `pos` - Right-bottom-front rectangle position
	// `dimensions` - POSITIVE rectangle dimensions in XY plane
	// Returns `true` if rect intersects with blocks
	// 	different than block_type::none, `false` otherwise
	bool collision_check_XY_rect(glm::vec3 pos, glm::vec2 dimensions);

private:
	block_type void_block = block_type::none;
	// expiration_queue_t expiration_queue;
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
