#include "world.hpp"

world_t::world_t(size_t width, size_t height, size_t depth)
	:width  { width  }
	,height { height }
	,depth  { depth  }
	,content(width,
			std::vector<std::vector<uint8_t>>(height,
				std::vector<uint8_t>(depth, 0)))
{ }

void world_t::update_buffers() {
	vertex_buffer.clear();
	// vertex_colors.clear();
	vertex_uvs.clear();

	for (size_t x = 0; x < content.size(); ++x) {
		for (size_t y = 0; y < content[x].size(); ++y) {
			for (size_t z = 0; z < content[x][y].size(); ++z) {
				if (content[x][y][z] == 0) continue;

				for (size_t i = 0; i < single_block_points_cnt; ++i) {
					const glm::vec3 pos(
						single_block_positions[3*i+0] + 2.0f * x,
						single_block_positions[3*i+1] + 2.0f * y,
						single_block_positions[3*i+2] + -2.0f * z
					);
					vertex_buffer.push_back(pos.x);
					vertex_buffer.push_back(pos.y);
					vertex_buffer.push_back(pos.z);

					// const glm::vec3 color(
					// 	single_block_colors[3*i+0],
					// 	single_block_colors[3*i+1],
					// 	single_block_colors[3*i+2]
					// );
					// vertex_colors.push_back(color.x);
					// vertex_colors.push_back(color.y);
					// vertex_colors.push_back(color.z);

					const glm::vec2 uv(
						single_block_uv[2*i+0],
						single_block_uv[2*i+1]
					);
					vertex_uvs.push_back(uv.x);
					vertex_uvs.push_back(uv.y);
				}
			}
		}
	}
}
