#include "world_buffer.hpp"

// block_type& world_buffer_t::get(glm::ivec3 pos) {
// 	if (!(0 <= pos.y && pos.y <= static_cast<int>(chunk_t::height)))
// 		return void_block;
//
// 	const glm::ivec2 chunk_pos(
// 		pos.x/chunk_t::width,
// 		pos.z/chunk_t::depth
// 	);
// 	chunk_t &chunk = chunks[chunk_pos];
//
// 	const int Mx = chunk_t::width;
// 	const int My = chunk_t::height;
// 	const int Mz = chunk_t::depth;
//
// 	glm::tvec3<size_t, glm::highp> block_pos(
// 			(pos.x%Mx+Mx)%Mx,
// 			(pos.y%My+My)%My,
// 			(pos.z%Mz+Mz)%Mz
// 		);
//
// 	return chunk.content
// 		[block_pos.x]
// 		[block_pos.y]
// 		[block_pos.z];
// }
