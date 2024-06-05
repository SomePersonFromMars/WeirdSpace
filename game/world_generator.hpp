#pragma once
#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP

#include <random>
#include <glm/glm.hpp>
#include "world_buffer.hpp"
#include "chunk.hpp"
#include "map_generator/noise.hpp"
#include "map_generator/map_storage.hpp"

#include <useful.hpp>

struct world_generator_t {
	world_generator_t(
		map_storage_t &map_storage,
		world_buffer_t &buffer);

    void load_settings();

	void gen_chunk(const glm::ivec2 &chunk_pos);

	float noise_pos_mult = 1.0/512.0*8.0;

private:
    void place_cactus(chunk_t &chunk, int x, int y, int z);

	map_storage_t &map_storage;
	world_buffer_t &buffer;
	cyclic_noise_t noise;
    std::mt19937 random_generator;
    int terrain_height;
};

#endif
