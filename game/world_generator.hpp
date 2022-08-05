#pragma once
#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP

#include <cstdio>
#include <glm/glm.hpp>
#include "world_buffer.hpp"
#include "chunk.hpp"
#include "useful.hpp"
#include "noise.hpp"

struct world_generator_t {
	world_generator_t(world_buffer_t &buffer);
	void gen_chunk(const glm::ivec2 &chunk_pos);

	float noise_pos_mult = 1.0/512.0*8.0;

private:
	world_buffer_t &buffer;
	cyclic_noise_t noise;
};

#endif
