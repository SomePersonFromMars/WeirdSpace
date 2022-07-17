#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <map>
#include "utils/useful.hpp"
#include "chunk.hpp"

struct world_buffer_t {
	std::map<glm::ivec2, chunk_t, ivec2_cmp_t> chunks;

	inline block_type& get(glm::ivec3 pos);

private:
	block_type void_block = block_type::none;
};

inline block_type& world_buffer_t::get(glm::ivec3 pos) {
	return 0 <= pos.y && pos.y <= static_cast<int>(chunk_t::height) ?
		chunks[glm::ivec2(
			pos.x/chunk_t::width,
			pos.z/chunk_t::depth
		)].content AT3_M(
				pos.x, pos.y, pos.z,
				chunk_t::width, chunk_t::height, chunk_t::depth
			)
		: void_block;
}

#endif
