// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "chunk.hpp"

const glm::ivec3 chunk_content_t::DIMENSIONS = { WIDTH, HEIGHT, DEPTH };

chunk_content_t::chunk_content_t()
{
	for (size_t x = 0; x < WIDTH; ++x)
		for (size_t y = 0; y < HEIGHT; ++y)
			for (size_t z = 0; z < DEPTH; ++z)
				storage[x][y][z] = block_type::none;
}
