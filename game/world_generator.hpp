#pragma once
#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP

#include <cstdio>
#include <map>
#include <glm/glm.hpp>
#include "world_buffer.hpp"
#include "chunk.hpp"
#include "utils/useful.hpp"
#include "noise.hpp"

struct world_generator_t {
	world_generator_t(world_buffer_t &buffer);
	void gen_chunk(int chunk_x, int global_x);

private:
	world_buffer_t &buffer;
};

#endif
