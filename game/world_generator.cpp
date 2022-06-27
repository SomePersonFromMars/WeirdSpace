#include "world_generator.hpp"

void world_generator_t::gen_chunk(int chunk_x, int global_x) {
	chunk_t &chunk = chunks[chunk_x];
	for (int x = 0; x < static_cast<int>(chunk.width); ++x) {
		for (int z = 0; z < static_cast<int>(chunk.depth); ++z) {
			const int y = (x/2+z)%chunk.height;
			chunk.content AT3(x, y, z) = 1;
		}
	}
}
