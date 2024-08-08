// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "world_buffer.hpp"
#include "settings.hpp"
#include "useful.hpp"

void world_buffer_t::load_settings() {
    width = global_settings.map_width_in_units * global_settings.map_unit_resolution / chunk_content_t::WIDTH;
    height = 1;
    depth = global_settings.map_height_in_units * global_settings.map_unit_resolution / chunk_content_t::DEPTH;
}

// TODO: Maintain preprocessing, that does not need to be cleared.
void world_buffer_t::preprocessing_system_load_settings_reload() {
    std::size_t max_active_elements_cnt = calculate_min_max_preprocessed_chunks_cnt_for_render_distance(global_settings.render_distance);

    expiration_queue.clear_and_load_settings(global_settings.max_preprocessed_chunks_cnt, max_active_elements_cnt);

    chunks_info.clear();
    for (auto &[chunk_rendering, buffer_pos_XZ] : preprocessed_chunks_buffer) {
        chunk_rendering.deinit_gl();
    }
    preprocessed_chunks_buffer.clear();
    preprocessed_chunks_buffer.resize(global_settings.max_preprocessed_chunks_cnt);
    for (auto &[chunk_rendering, buffer_pos_XZ] : preprocessed_chunks_buffer) {
        buffer_pos_XZ = INVALID_IVEC2;
    }
}

void world_buffer_t::refresh_preprocessing_tasks(const camera_t &camera) {
    expiration_queue.push_back_all_active_elements_to_queue();
    chunks_to_render_priority_stack.clear();

    const float render_distance = global_settings.render_distance;
    const float visibility_distance = render_distance * static_cast<float>(chunk_content_t::WIDTH);
    const int off = std::ceil(render_distance);
    const int origin_x = camera.get_position().x / static_cast<float>(chunk_content_t::WIDTH);
    const int origin_z = camera.get_position().z / static_cast<float>(chunk_content_t::DEPTH);

    // Iterate over new active chunks and assign priorities only to them.
    for (
        int unnormalised_x = origin_x-off, coverage = 0;
        unnormalised_x <= origin_x+off and coverage < get_buffer_width();
        ++unnormalised_x, ++coverage)
    {
        for (int z = origin_z-off; z <= origin_z+off; ++z) {
            const int x = mod(unnormalised_x, get_buffer_width());
            if (z < 0 or z >= get_buffer_depth())
                continue;

            const glm::ivec2 buffer_pos_XZ = {x, z};
            const glm::vec3 buffer_pos_XYZ = {
                static_cast<float>(x),
                0.0f,
                static_cast<float>(z)
            };

            const float priority
                = chunk_rendering_t::calculate_preprocessing_priority(
                        buffer_pos_XYZ,
                        get_world_width(),
                        camera,
                        visibility_distance);

            if (not (priority > 0.0f))
                continue;

            // SMALLOPT: The element goes unnecessarily to the QUEUE_LIST first.
            const std::size_t id = get_chunk_rendering_id_possibly_freeing_old(buffer_pos_XZ);
            assert(expiration_queue.is_element_in_queue(id));
            expiration_queue.push_back_element_to_active_list_from_queue(id);
            chunks_info[buffer_pos_XZ].preprocessing_priority = priority;
            if (not preprocessed_chunks_buffer[id].first.is_ready_to_render())
                chunks_to_render_priority_stack.push_back({priority, buffer_pos_XZ});
        }
    }

    std::sort(chunks_to_render_priority_stack.begin(), chunks_to_render_priority_stack.end(),
            [](const chunk_with_priority_t &a, const chunk_with_priority_t &b) {
            return a.priority < b.priority;
            });
}

bool world_buffer_t::preprocess_most_urgent_chunk() {
    while (not chunks_to_render_priority_stack.empty()) {
        const auto [priority, buffer_pos_XZ] = chunks_to_render_priority_stack.back();
        chunks_to_render_priority_stack.pop_back();

        const std::size_t id = get_chunk_rendering_id(buffer_pos_XZ);
        chunk_rendering_t &chunk_rendering = preprocessed_chunks_buffer[id].first;

        if (not chunk_rendering.is_ready_to_render()) {
            const chunk_content_t &chunk_content = chunks_contents[buffer_pos_XZ];
            chunk_rendering.preprocess_on_cpu(chunk_content);
            chunk_rendering.init_gl();
            chunk_rendering.send_preprocessed_to_gpu();
            preprocessed_chunks_buffer[id].second = buffer_pos_XZ;
            break;
        }
    }

    return not chunks_to_render_priority_stack.empty();
}

void world_buffer_t::draw_cyclicly_if_visible(
        const glm::mat4 &projection_matrix,
        const glm::mat4 &view_matrix,
        const shader_A_fragment_common_uniforms_t &shader_A_fragment_common_uniforms,
        const frustum_t &camera_frustum
        )
{
    expiration_queue.for_each_active_element_id(
            [ this,
            &projection_matrix,
            &view_matrix,
            &shader_A_fragment_common_uniforms,
            &camera_frustum ](std::size_t i) {

            const auto &[chunk_rendering, buffer_pos_XZ] = preprocessed_chunks_buffer[i];
            if (not chunk_rendering.is_ready_to_render())
                return;

            const glm::vec3 buffer_pos_XYZ = {
            static_cast<float>(buffer_pos_XZ.x),
            0.0f,
            static_cast<float>(buffer_pos_XZ.y)
            };

            chunk_rendering.draw_cyclicly_if_visible(
                    projection_matrix,
                    view_matrix,
                    shader_A_fragment_common_uniforms,
                    buffer_pos_XYZ,
                    get_world_width(),
                    camera_frustum);
            }
    );
}

bool world_buffer_t::collision_check_XY_rect(
        glm::vec3 pos, glm::vec2 dimensions) {
    const glm::vec2 pos_end(pos.x+dimensions.x, pos.y+dimensions.y);

    glm::vec2 iterations_cnt(
            std::ceil(dimensions.x)+1,
            std::ceil(dimensions.y)+1
            );
    if (pos_end.x == std::floor(pos_end.x))
        iterations_cnt.x -= 1;
    if (pos_end.y == std::floor(pos_end.y))
        iterations_cnt.y -= 1;

    for (float dx = 0, j = 0; j < iterations_cnt.x; j += 1) {
        for (float dy = 0, i = 0; i < iterations_cnt.y; i += 1) {
            const glm::ivec3 wanted_block_pos(
                    std::floor(pos.x+dx),
                    std::floor(pos.y+dy),
                    std::floor(pos.z)
                    );
            if (get(wanted_block_pos) != block_type::none)
                return true;

            dy += 1;
            if (dy > dimensions.y)
                dy = dimensions.y;
        }

        dx += 1;
        if (dx > dimensions.x)
            dx = dimensions.x;
    }
    return false;
}

std::size_t world_buffer_t::get_chunk_rendering_id(glm::ivec2 buffer_pos_XZ) const {
    const auto it = chunks_info.find(buffer_pos_XZ);
    assert(it != chunks_info.end());
    return it->second.id_in_expiration_queue;
}

std::size_t world_buffer_t::get_chunk_rendering_id_possibly_freeing_old(glm::ivec2 buffer_pos_XZ) {
    auto [chunk_info_it, emplaced] = chunks_info.try_emplace(buffer_pos_XZ, chunk_info_t());
    chunk_info_t &chunk_info = chunk_info_it->second;
    std::size_t &id = chunk_info.id_in_expiration_queue;

    if (emplaced) {
        const std::size_t expired_id = expiration_queue.get_oldest_queue_element_id();
        {
            const auto it = chunks_info.find(preprocessed_chunks_buffer[expired_id].second);
            if (it != chunks_info.end())
                chunks_info.erase(it);
        }
        preprocessed_chunks_buffer[expired_id].first.deinit_gl();
        preprocessed_chunks_buffer[expired_id].first.clear_cpu_preprocessing_data();
        preprocessed_chunks_buffer[expired_id].second = INVALID_IVEC2;

        id = expired_id;
        expiration_queue.push_back_element_to_queue(id);
        chunk_info.id_in_expiration_queue = id;
    }
    assert(expiration_queue.is_correct_element_id(id));
    return id;
}
