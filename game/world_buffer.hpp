// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef WORLD_BUFFER_HPP
#define WORLD_BUFFER_HPP

#include <utility>
#include <vector>
#include <map>

#include <expiration_queue.hpp>
#include "chunk.hpp"
#include "camera.hpp"

struct world_buffer_t {
    // Methods
    void load_settings();
    void preprocessing_system_load_settings_reload();

    inline int get_buffer_width() const;
    inline int get_buffer_height() const;
    inline int get_buffer_depth() const;
    inline int get_world_width() const;
    inline int get_world_height() const;
    inline int get_world_depth() const;

    inline block_type& get(glm::ivec3 pos);
    inline chunk_content_t& get_chunk_content(glm::ivec2 buffer_pos_XZ);
    inline const chunk_info_t* get_chunk_info_ptr(glm::ivec2 buffer_pos_XZ) const;

    void refresh_preprocessing_tasks(const camera_t &camera);
    bool preprocess_most_urgent_chunk();
    void draw_cyclicly_if_visible(
            const glm::mat4 &projection_matrix,
            const glm::mat4 &view_matrix,
            const shader_A_fragment_common_uniforms_t &shader_A_fragment_common_uniforms,
            const frustum_t &camera_frustum
            );

    // `pos` - Right-bottom-front rectangle position
    // `dimensions` - POSITIVE rectangle dimensions in XY plane
    // Returns `true` if rect intersects with blocks
    // 	different than block_type::none, `false` otherwise
    bool collision_check_XY_rect(glm::vec3 pos, glm::vec2 dimensions);

    inline static std::size_t calculate_min_max_preprocessed_chunks_cnt_for_render_distance(float render_distance);

private:
    // Methods
    std::size_t get_chunk_rendering_id(glm::ivec2 buffer_pos_XZ) const;
    std::size_t get_chunk_rendering_id_possibly_freeing_old(glm::ivec2 buffer_pos_XZ);

    // Internal structures
    struct chunk_with_priority_t {
        float priority;
        glm::ivec2 buffer_pos_XZ;
    };

    // Small fields
    // World dimensions in chunks
    int width = 0;
    int height = 0;
    int depth = 0;

    block_type void_block = block_type::none;

    // Big data structures
    std::map<glm::ivec2, chunk_content_t, vec2_cmp_t<int>> chunks_contents;
    std::map<glm::ivec2, chunk_info_t, vec2_cmp_t<int>> chunks_info;

    expiration_queue_t expiration_queue;
    std::vector<chunk_with_priority_t> chunks_to_render_priority_stack;
    std::vector<std::pair<chunk_rendering_t, glm::ivec2>> preprocessed_chunks_buffer;
};

inline int world_buffer_t::get_buffer_width() const {
    return width;
}
inline int world_buffer_t::get_buffer_height() const {
    return height;
}
inline int world_buffer_t::get_buffer_depth() const {
    return depth;
}
inline int world_buffer_t::get_world_width() const {
    return width*chunk_content_t::WIDTH;
}
inline int world_buffer_t::get_world_height() const {
    return height*chunk_content_t::HEIGHT;
}
inline int world_buffer_t::get_world_depth() const {
    return depth*chunk_content_t::DEPTH;
}

inline block_type& world_buffer_t::get(glm::ivec3 pos) {
    return 0 <= pos.y && pos.y < static_cast<int>(chunk_content_t::HEIGHT) ?
        chunks_contents[glm::ivec2(
                floor_div(pos.x, static_cast<int>(chunk_content_t::WIDTH)) % width,
                floor_div(pos.z, static_cast<int>(chunk_content_t::DEPTH))
                )].storage AT3_M(
                    pos.x, pos.y, pos.z,
                    chunk_content_t::WIDTH, chunk_content_t::HEIGHT, chunk_content_t::DEPTH
                    )
                : void_block;
}
inline chunk_content_t& world_buffer_t::get_chunk_content(glm::ivec2 buffer_pos_XZ) {
    return chunks_contents[buffer_pos_XZ];
}
inline const chunk_info_t* world_buffer_t::get_chunk_info_ptr(glm::ivec2 buffer_pos_XZ) const {
    const auto it = chunks_info.find(buffer_pos_XZ);
    if (it == chunks_info.end())
        return nullptr;
    else {
        const chunk_info_t &chunk_info = it->second;
        return &chunk_info;
    }
}

inline std::size_t world_buffer_t::calculate_min_max_preprocessed_chunks_cnt_for_render_distance(float render_distance) {
    return pow_two(2*static_cast<std::size_t>(std::ceil(render_distance))+1);
}

#endif
