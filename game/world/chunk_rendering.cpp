// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "chunk.hpp"

#include <cstdio>

#include <utilities/texture_loader.hpp>
#include <utilities/useful.hpp>
#include <settings/settings.hpp>

#include <utilities/math/bounding_volume.hpp>
#include <camera/camera.hpp>
#include "block_model.hpp"

#ifdef _MSC_VER
#   include <intrin.h>
#   define __builtin_popcount __popcnt
#endif

shader_world_t *chunk_rendering_t::pshader;
GLuint chunk_rendering_t::texture_id;
GLuint chunk_rendering_t::block_model_uniform_buffer_id;

void chunk_rendering_t::init_gl_static(shader_world_t *shader_ptr) {
    chunk_rendering_t::pshader = shader_ptr;

    // Load the combined texture
    texture_id = load_texture(TEXTURE_BLOCKS_COMBINED_PATH);

    // Generate, fill and bind the uniform buffer object
    glGenBuffers(1, &block_model_uniform_buffer_id);
    glBindBuffer(GL_UNIFORM_BUFFER, block_model_uniform_buffer_id);
    // Allocate the buffer
    glBufferData(GL_UNIFORM_BUFFER,
            sizeof(block_model::POSITIONS)+sizeof(block_model::UVS)+sizeof(block_model::NORMALS),
            nullptr, GL_STATIC_DRAW);

    // Fill the buffer
    glBufferSubData(GL_UNIFORM_BUFFER,
            0, sizeof(block_model::POSITIONS), &block_model::POSITIONS[0]);
    glBufferSubData(GL_UNIFORM_BUFFER,
            sizeof(block_model::POSITIONS), sizeof(block_model::UVS), &block_model::UVS[0]);
    glBufferSubData(GL_UNIFORM_BUFFER,
            sizeof(block_model::POSITIONS)+sizeof(block_model::UVS), sizeof(block_model::NORMALS),
            &block_model::NORMALS[0]);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Bind the whole buffer to GL_UNIFORM_BUFFER indexed buffer
    // at index shader.block_model_uniform_binding_point
    glBindBufferRange(GL_UNIFORM_BUFFER,
            pshader->block_model_uniform_binding_point,
            block_model_uniform_buffer_id, 0,
            sizeof(block_model::POSITIONS)+sizeof(block_model::UVS)+sizeof(block_model::NORMALS));
}

void chunk_rendering_t::deinit_gl_static() {
    glDeleteTextures(1, &texture_id);
    glDeleteBuffers(1, &block_model_uniform_buffer_id);
}

void chunk_rendering_t::init_gl() {
    // VAO
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    glGenBuffers(1, &positions_instanced_buffer_id);
    glGenBuffers(1, &blocks_types_instanced_buffer_id);
    glGenBuffers(1, &faces_types_instanced_buffer_id);

    // Initialize VBOs with single instance data
    // There is none

    // Add shader vertex attributes to the VAO
    // Single vertex attributes
    // There are none

    // Instanced data
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(0, 1);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, faces_types_instanced_buffer_id);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 0, (void*)0);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void chunk_rendering_t::deinit_gl() {
    glDeleteBuffers(1,  &positions_instanced_buffer_id);
    glDeleteBuffers(1,  &blocks_types_instanced_buffer_id);
    glDeleteBuffers(1,  &faces_types_instanced_buffer_id);

    glDeleteVertexArrays(1, &vao_id);
}

float chunk_rendering_t::calculate_preprocessing_priority(
        const glm::vec3 &buffer_chunk_position_XYZ,
        const float      world_width,
        const camera_t &camera,
        const float     visibility_distance) {

    const glm::vec3 base_chunk_pos_world_coords_XYZ = {
        buffer_chunk_position_XYZ.x * chunk_content_t::WIDTH,
        buffer_chunk_position_XYZ.y * chunk_content_t::HEIGHT,
        buffer_chunk_position_XYZ.z * chunk_content_t::DEPTH };

    const float A_priority = calculate_single_preprocessing_priority(
            base_chunk_pos_world_coords_XYZ - glm::vec3(world_width, 0, 0),
            camera,
            visibility_distance);

    const float B_priority = calculate_single_preprocessing_priority(
            base_chunk_pos_world_coords_XYZ,
            camera,
            visibility_distance);

    const float C_priority = calculate_single_preprocessing_priority(
            base_chunk_pos_world_coords_XYZ + glm::vec3(world_width, 0, 0),
            camera,
            visibility_distance);

    return std::max(std::max(A_priority, B_priority), C_priority);
}

void chunk_rendering_t::clear_cpu_preprocessing_data() {
    positions_instanced_buffer.clear();
    blocks_types_instanced_buffer.clear();
    faces_types_instanced_buffer.clear();
    ready_to_render = false;
}

void chunk_rendering_t::preprocess_on_cpu(const chunk_content_t &content) {
    float average_faces_visible = 0;
    float visible_blocks_cnt = 0;

    for (size_t x = 0; x < content.WIDTH; ++x) {
        for (size_t y = 0; y < content.HEIGHT; ++y) {
            for (size_t z = 0; z < content.DEPTH; ++z) {
                if (content.storage[x][y][z] == block_type::none) continue;

                uint8_t faces_mask = 0x3f;

                if (x > 0) {
                    if (content.storage[x-1][y][z] != block_type::none)
                        faces_mask &= ~(1<<3);
                } else {
                    if (content.neighbors[0] != nullptr
                            && content.neighbors[0]->storage[content.WIDTH-1][y][z]
                            != block_type::none)
                        faces_mask &= ~(1<<3);
                }
                if (x < content.WIDTH-1) {
                    if (content.storage[x+1][y][z] != block_type::none)
                        faces_mask &= ~(1<<2);
                } else {
                    if (content.neighbors[1] != nullptr
                            && content.neighbors[1]->storage[0][y][z]
                            != block_type::none)
                        faces_mask &= ~(1<<2);
                }

                if (y > 0) {
                    if (content.storage[x][y-1][z] != block_type::none)
                        faces_mask &= ~(1<<4);
                } else {
                    // if (content.neighbors[2] != nullptr
                    // 	&& content.neighbors[2]->buffer[x][content.HEIGHT-1][z]
                    // 	!= block_type::none)
                    faces_mask &= ~(1<<4);
                }
                if (y < content.HEIGHT-1) {
                    if (content.storage[x][y+1][z] != block_type::none)
                        faces_mask &= ~(1<<1);
                } else {
                    if (content.neighbors[3] != nullptr
                            && content.neighbors[3]->storage[x][0][z]
                            != block_type::none)
                        faces_mask &= ~(1<<1);
                }

                if (z > 0) {
                    if (content.storage[x][y][z-1] != block_type::none)
                        faces_mask &= ~(1<<0);
                } else {
                    if (content.neighbors[4] != nullptr
                            && content.neighbors[4]->storage[x][y][content.DEPTH-1]
                            != block_type::none)
                        faces_mask &= ~(1<<0);
                }
                if (z < content.DEPTH-1) {
                    if (content.storage[x][y][z+1] != block_type::none)
                        faces_mask &= ~(1<<5);
                } else {
                    if (content.neighbors[5] != nullptr
                            && content.neighbors[5]->storage[x][y][0]
                            != block_type::none)
                        faces_mask &= ~(1<<5);
                }

                if (faces_mask == 0)
                    continue;

                average_faces_visible += __builtin_popcount(faces_mask);
                // average_faces_visible += 6;
                visible_blocks_cnt += 1;

                const glm::vec3 block_pos(
                        static_cast<float>(x),
                        static_cast<float>(y),
                        static_cast<float>(z)
                        );

                for (uint8_t i = 0; i < 6; ++i) {
                    if (!(faces_mask & (1<<i)))
                        continue;

                    positions_instanced_buffer.push_back(block_pos.x);
                    positions_instanced_buffer.push_back(block_pos.y);
                    positions_instanced_buffer.push_back(block_pos.z);

                    blocks_types_instanced_buffer.push_back(
                            static_cast<uint8_t>(content.storage[x][y][z]));

                    faces_types_instanced_buffer.push_back(i);
                }
            }
        }
    }

    // average_faces_visible /= visible_blocks_cnt;
    // PRINT_F(average_faces_visible);
    // PRINT_F(visible_blocks_cnt);
}

void chunk_rendering_t::send_preprocessed_to_gpu() {
    ready_to_render = not positions_instanced_buffer.empty();
    assert(is_ready_to_render());

    glBindBuffer(GL_ARRAY_BUFFER, positions_instanced_buffer_id);
    glBufferData(GL_ARRAY_BUFFER,
            positions_instanced_buffer.size()*sizeof(GLfloat),
            &positions_instanced_buffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, blocks_types_instanced_buffer_id);
    glBufferData(GL_ARRAY_BUFFER,
            blocks_types_instanced_buffer.size()*sizeof(GLubyte),
            &blocks_types_instanced_buffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, faces_types_instanced_buffer_id);
    glBufferData(GL_ARRAY_BUFFER,
            faces_types_instanced_buffer.size()*sizeof(GLubyte),
            &faces_types_instanced_buffer[0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void chunk_rendering_t::draw(
        const glm::mat4 &projection_matrix,
        const glm::mat4 &view_matrix,
        const glm::mat4 &model_matrix,
        const shader_A_fragment_common_uniforms_t &common_uniforms
        ) const {
    glUseProgram(pshader->program_id);

    glUniformMatrix4fv(pshader->model_matrix_uniform,
            1, GL_FALSE, &model_matrix[0][0]);
    glUniformMatrix4fv(pshader->view_matrix_uniform,
            1, GL_FALSE, &view_matrix[0][0]);
    glUniformMatrix4fv(pshader->projection_matrix_uniform,
            1, GL_FALSE, &projection_matrix[0][0]);

    pshader->common_fragment_uniforms_locations.send_values(common_uniforms);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(pshader->texture_sampler_uniform,
            0);

    glBindVertexArray(vao_id);
    glDrawArraysInstanced(GL_TRIANGLES,
            0, 6,
            positions_instanced_buffer.size()/3
            );
    glBindVertexArray(0);
}

float chunk_rendering_t::calculate_single_preprocessing_priority(
        const glm::vec3 &chunk_copy_world_position_XYZ,
        const camera_t &camera,
        const float visibility_distance) {
    glm::vec3 mid_chunk_pos =
        chunk_copy_world_position_XYZ +
        glm::vec3(
                chunk_content_t::WIDTH,
                chunk_content_t::HEIGHT,
                chunk_content_t::DEPTH
                ) * 0.5f;
    const glm::vec3 &camera_pos = camera.get_position();
    const glm::vec3 off = mid_chunk_pos - camera_pos;
    const float off_len = std::sqrt(off.x*off.x + off.y*off.y + off.z*off.z);

    const float priority
        = 1.0f
        // FIXME: This multiplier does not seem to work properly. Would probably be simpler, if chunks were cubes.
        - std::min(
                off_len / (visibility_distance + 2.122f*static_cast<float>(chunk_content_t::WIDTH)),
                1.0f
                );

    return priority;
}

void chunk_rendering_t::draw_cyclicly_if_visible(
        const glm::mat4 &projection_matrix,
        const glm::mat4 &view_matrix,
        const shader_A_fragment_common_uniforms_t &common_uniforms,
        const glm::vec3 &buffer_chunk_position_XYZ,
        const float      world_buffer_width,
        const frustum_t &camera_frustum) const {

    const glm::vec3 base_chunk_pos_world_coords_XYZ = {
        buffer_chunk_position_XYZ.x * chunk_content_t::WIDTH,
        buffer_chunk_position_XYZ.y * chunk_content_t::HEIGHT,
        buffer_chunk_position_XYZ.z * chunk_content_t::DEPTH };

    const bool A_visible = draw_single_copy_if_visible(
            projection_matrix,
            view_matrix,
            common_uniforms,
            base_chunk_pos_world_coords_XYZ - glm::vec3(world_buffer_width, 0, 0),
            camera_frustum);

    const bool B_visible = draw_single_copy_if_visible(
            projection_matrix,
            view_matrix,
            common_uniforms,
            base_chunk_pos_world_coords_XYZ,
            camera_frustum);

    const bool C_visible = draw_single_copy_if_visible(
            projection_matrix,
            view_matrix,
            common_uniforms,
            base_chunk_pos_world_coords_XYZ + glm::vec3(world_buffer_width, 0, 0),
            camera_frustum);

    rendering_enabled_info = A_visible or B_visible or C_visible;
}

bool chunk_rendering_t::draw_single_copy_if_visible(
        const glm::mat4 &projection_matrix,
        const glm::mat4 &view_matrix,
        const shader_A_fragment_common_uniforms_t &common_uniforms,
        const glm::vec3 &chunk_copy_world_position_XYZ,
        const frustum_t &camera_frustum) const {

    const glm::mat4 model_matrix = {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        glm::vec4(chunk_copy_world_position_XYZ, 1.0f)
    };

    const AABB_t bounding_box(
            chunk_copy_world_position_XYZ,
            chunk_copy_world_position_XYZ + static_cast<glm::vec3>(chunk_content_t::DIMENSIONS));

    if (bounding_box.is_on_frustum(camera_frustum)) {
        draw(
                projection_matrix,
                view_matrix,
                model_matrix,
                common_uniforms);
        return true;
    } else
        return false;
}
