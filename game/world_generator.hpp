#pragma once
#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP

#include <cstdio>
#include <map>
#include <glm/glm.hpp>
#include "chunk.hpp"
#include "utils/useful.hpp"
#include "noise.hpp"

struct world_generator_t {
	world_generator_t() = default;

	std::map<int, chunk_t> chunks;

	void gen_chunk(int chunk_x, int global_x);
};

#endif
