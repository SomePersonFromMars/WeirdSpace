#include "world_generator.hpp"

world_generator_t::world_generator_t(world_buffer_t &buffer)
	:buffer{buffer}
{  }

void world_generator_t::gen_chunk(int chunk_x, int global_x) {
	chunk_t &chunk = buffer.chunks[chunk_x];

	for (int x = 0; x < static_cast<int>(chunk.width); ++x) {
		for (int z = 0; z < static_cast<int>(chunk.depth); ++z) {
			const float p =
				// glm::clamp(
					(1.0f+perlin(
					(float)x/(float)chunk.width*2.0f,
					(float)z/(float)chunk.depth*2.0f))/2.0f ;
					// , 0.0f, 1.0f);
			printf("%f\n", p);
			const int y = ( p * static_cast<float>(chunk.height) );
			// const int y = (x/2+z)%chunk.height;
			chunk.content AT3(x, y, z) = block_type::sand;
		}
	}
}
