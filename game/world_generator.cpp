#include "world_generator.hpp"
#include <random>

world_generator_t::world_generator_t(
		map_storage_t &map_storage,
		world_buffer_t &buffer)
	:map_storage{map_storage}
	,buffer{buffer}
{
}

void world_generator_t::load_settings() {
	noise.reseed(1234);
	noise.border_end = float(buffer.get_buffer_width()*chunk_t::WIDTH) * noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= float(chunk_t::WIDTH)*0.5 * noise_pos_mult;
    random_generator = std::mt19937(1234);
    terrain_height = global_settings.terrain_height_in_blocks;
    assert(terrain_height <= chunk_t::HEIGHT);
}

void world_generator_t::gen_chunk(const glm::ivec2 &chunk_pos) {
	chunk_t &chunk = buffer.chunks[chunk_pos];

	for (int x = 0; x < static_cast<int>(chunk.WIDTH); ++x) {
		for (int z = 0; z < static_cast<int>(chunk.DEPTH); ++z) {

			// const float p =
			// 	(1.0f+perlin(
			// 	(float)(x + chunk_pos.x*chunk.WIDTH)/(float)chunk.WIDTH*8.0f,
			// 	(float)(z + chunk_pos.y*chunk.DEPTH)/(float)chunk.DEPTH*8.0f
			// 	))/2.0f ;
			// const float p = noise.octave2D_01(
			// 	(float)(x + chunk_pos.x*chunk.WIDTH)*noise_pos_mult,
			// 	(float)(z + chunk_pos.y*chunk.DEPTH)*noise_pos_mult,
			// 	2
			// );
			const float p = (float)map_storage.get_component_value(
				z + chunk_pos.y*chunk.DEPTH,
				x + chunk_pos.x*chunk.WIDTH,
				3) / 255.0f;

			// int y = ( (p*3.0-1.0) * static_cast<float>(terrain_height) );
			int y = ( (p) * static_cast<float>(terrain_height) ) + 1;
			y = std::min(y, terrain_height-1);

            if (
                std::uniform_int_distribution<int>(1, 1000)(random_generator) <= 1 and
                y >= terrain_height/2)
                place_cactus(chunk, x, y+1, z);

			while (y >= 0)
                chunk.set_block(x, y--, z, block_type::sand);
		}
	}
}

void world_generator_t::place_cactus(chunk_t &chunk, int x, int y, int z) {
    chunk.set_block(x, y, z, block_type::cactus);
    chunk.set_block(x, y+1, z, block_type::cactus);

    const int direction = std::uniform_int_distribution<int>(0, 1)(random_generator);

    switch (direction) {
        case 0:
            chunk.set_block(x-1, y+1, z, block_type::cactus);
            chunk.set_block(x+1, y+1, z, block_type::cactus);
            chunk.set_block(x-1, y+2, z, block_type::cactus);
            chunk.set_block(x+1, y+2, z, block_type::cactus);
            break;
        case 1:
            chunk.set_block(x, y+1, z-1, block_type::cactus);
            chunk.set_block(x, y+1, z+1, block_type::cactus);
            chunk.set_block(x, y+2, z-1, block_type::cactus);
            chunk.set_block(x, y+2, z+1, block_type::cactus);
            break;
    }
}
