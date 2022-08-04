#include "world_generator.hpp"

world_generator_t::world_generator_t(world_buffer_t &buffer)
	:buffer{buffer}
{  }

void world_generator_t::gen_chunk(const glm::ivec2 &chunk_pos) {
	chunk_t &chunk = buffer.chunks[chunk_pos];

	for (int x = 0; x < static_cast<int>(chunk.WIDTH); ++x) {
		for (int z = 0; z < static_cast<int>(chunk.DEPTH); ++z) {
			const float p =
				(1.0f+perlin(
				(float)(x + chunk_pos.x*chunk.WIDTH)/(float)chunk.WIDTH*8.0f,
				(float)(z + chunk_pos.y*chunk.DEPTH)/(float)chunk.DEPTH*8.0f
				))/2.0f ;
			// int y = ( (p*3.0-1.0) * static_cast<float>(chunk.HEIGHT) );
			int y = ( (p) * static_cast<float>(chunk.HEIGHT) );
			y = std::min(y, chunk.HEIGHT-1);
			while (y > 0)
				chunk.content AT3(x, y, z) = block_type::sand, --y;
		}
	}
}
