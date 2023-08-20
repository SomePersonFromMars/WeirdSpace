#include "world_generator.hpp"

world_generator_t::world_generator_t(
		map_storage_t &map_storage,
		world_buffer_t &buffer)
	:map_storage{map_storage}
	,buffer{buffer}
{
	noise.reseed(1234);
	noise.border_end = float(buffer.width*chunk_t::WIDTH) * noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= float(chunk_t::WIDTH)*0.5 * noise_pos_mult;
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

			// int y = ( (p*3.0-1.0) * static_cast<float>(chunk.HEIGHT) );
			int y = ( (p) * static_cast<float>(chunk.HEIGHT) ) + 1;
			y = std::min(y, chunk.HEIGHT-1);
			while (y > 0)
				chunk.content AT3(x, y, z) = block_type::sand, --y;
		}
	}
}
