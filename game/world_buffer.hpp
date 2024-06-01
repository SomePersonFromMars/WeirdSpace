#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <map>
#include <functional>

#include <expiration_queue.hpp>
#include "chunk.hpp"

struct world_buffer_t {
    // Data
	std::map<glm::ivec2, chunk_t, vec2_cmp_t<int>> chunks;

    // Methods
    void load_settings();

    inline int get_buffer_width() const;
    inline int get_buffer_height() const;
    inline int get_buffer_depth() const;
    inline int get_world_width() const;
    inline int get_world_height() const;
    inline int get_world_depth() const;

	inline block_type& get(glm::ivec3 pos);
	inline void for_each_active_chunk(const std::function<void(chunk_t&)> f);

	// `pos` - Right-bottom-front rectangle position
	// `dimensions` - POSITIVE rectangle dimensions in XY plane
	// Returns `true` if rect intersects with blocks
	// 	different than block_type::none, `false` otherwise
	bool collision_check_XY_rect(glm::vec3 pos, glm::vec2 dimensions);

private:
	// World dimensions in chunks
    int width = 0;
    int height = 0;
    int depth = 0;

	block_type void_block = block_type::none;
	// expiration_queue_t expiration_queue;
};

inline int world_buffer_t::get_buffer_width() const {
    return width;
}
inline int world_buffer_t::get_buffer_height() const {
    return height;
}
inline int world_buffer_t::get_buffer_depth() const {
    return depth;
}
inline int world_buffer_t::get_world_width() const {
    return width*chunk_t::WIDTH;
}
inline int world_buffer_t::get_world_height() const {
    return height*chunk_t::HEIGHT;
}
inline int world_buffer_t::get_world_depth() const {
    return depth*chunk_t::DEPTH;
}

inline block_type& world_buffer_t::get(glm::ivec3 pos) {
	return 0 <= pos.y && pos.y < static_cast<int>(chunk_t::HEIGHT) ?
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
