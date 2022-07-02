#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <map>
#include "utils/useful.hpp"
#include "chunk.hpp"

struct world_buffer_t {
	std::map<glm::ivec2, chunk_t, ivec2_cmp_t> chunks;
};

#endif
