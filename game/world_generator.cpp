#include "world_generator.hpp"

world_generator_t::world_generator_t(world_buffer_t &buffer)
	:buffer{buffer}
{  }

void world_generator_t::gen_chunk(const glm::ivec2 &chunk_pos) {
	chunk_t &chunk = buffer.chunks[chunk_pos];

	for (int x = 0; x < static_cast<int>(chunk.width); ++x) {
		for (int z = 0; z < static_cast<int>(chunk.depth); ++z) {
			const float p =
				(1.0f+perlin(
				(float)(x + chunk_pos.x*chunk.width)/(float)chunk.width*2.0f,
				(float)(z + chunk_pos.y*chunk.depth)/(float)chunk.depth*2.0f
				))/2.0f ;
			// printf("%f\n", p);
			const int y = ( p * static_cast<float>(chunk.height) );
			// const int y = (x/2+z)%chunk.height;
			chunk.content AT3(x, y, z) = block_type::sand;
		}
	}
}
