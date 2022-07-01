#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <map>
#include "chunk.hpp"

struct world_buffer_t {
	std::map<int, chunk_t> chunks;
};

#endif
