// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <utilities/useful.hpp>
#include "shader_world.hpp"
#include <common_shading/shader_A.hpp>
#include <camera/camera.hpp>
#include <utilities/math/geometry.hpp>

enum class block_type : uint8_t {
    none = 0,
    sand = 1,
    brick,
    cactus,

    // Remember to update BLOCKS_CNT manually
    // in the `shader_world_vertex.glsl`!
    cnt
};

struct chunk_info_t {
    // This variable will be out-of-date when
    // chunk stays in expiration queue and
    // is not in the reach of updated render
    // priorities.
    float preprocessing_priority = 0.0f;

    std::size_t id_in_expiration_queue = INVALID_ID;
};

struct chunk_content_t {
    // Settings
    // TODO: Make it a cube.
    static constexpr int WIDTH = 32;
    static constexpr int HEIGHT = 128;
    static constexpr int DEPTH = 32;
    static const glm::ivec3 DIMENSIONS;

    // Methods
    chunk_content_t();
    inline void set_block(int x, int y, int z, block_type type);

    // Fields
    block_type storage[WIDTH][HEIGHT][DEPTH];
    // Neighbors order:
    // 0,  1,  2,  3,  4,  5
    // -x, +x, -y, +y, -z, +z
    const chunk_content_t *neighbors[6] { };
};

struct chunk_rendering_t {
    // Methods
    static void init_gl_static(shader_world_t *pshader);
    static void deinit_gl_static();
    void init_gl();
    void deinit_gl();

    static float calculate_preprocessing_priority(
            const glm::vec3 &buffer_chunk_position_XYZ,
            const float      world_buffer_width,
            const camera_t  &camera,
            const float      max_visibility_priority
            );
    void clear_cpu_preprocessing_data();
    void preprocess_on_cpu(const chunk_content_t &content);
    void send_preprocessed_to_gpu();

    inline float get_preprocessing_priority() const;
    inline bool is_ready_to_render() const;
    inline bool is_rendering_enabled() const;

    void draw(
            const glm::mat4 &projection_matrix,
            const glm::mat4 &view_matrix,
            const glm::mat4 &model_matrix,
            const shader_A_fragment_common_uniforms_t &common_uniforms
            ) const;

    void draw_cyclicly_if_visible(
            const glm::mat4 &projection_matrix,
            const glm::mat4 &view_matrix,
            const shader_A_fragment_common_uniforms_t &common_uniforms,
            const glm::vec3 &buffer_chunk_position_XYZ,
            const float      world_buffer_width,
            const frustum_t &camera_frustum
            ) const;

    private:
    // Methods
    static float calculate_single_preprocessing_priority(
            const glm::vec3 &chunk_copy_world_position_XYZ,
            const camera_t  &camera,
            const float      max_visibility_priority
            );
    bool draw_single_copy_if_visible(
            const glm::mat4 &projection_matrix,
            const glm::mat4 &view_matrix,
            const shader_A_fragment_common_uniforms_t &common_uniforms,
            const glm::vec3 &chunk_copy_world_position_XYZ,
            const frustum_t &camera_frustum
            ) const;

    // Fields
    bool ready_to_render = false;
    mutable bool rendering_enabled_info = true;

    // Static shader related data
    static shader_world_t *pshader;
    static GLuint texture_id; // All blocks combined texture
    static GLuint block_model_uniform_buffer_id;

    // Per chunk shader data
    GLuint vao_id;

    GLuint positions_instanced_buffer_id;
    GLuint blocks_types_instanced_buffer_id;
    GLuint faces_types_instanced_buffer_id;

    std::vector<float> positions_instanced_buffer;
    std::vector<uint8_t> blocks_types_instanced_buffer;
    std::vector<uint8_t> faces_types_instanced_buffer;
};

inline void chunk_content_t::set_block(int x, int y, int z, block_type type) {
    if (0 <= x and x < WIDTH and 0 <= y and y < HEIGHT and 0 <= z and z < DEPTH)
        storage[x][y][z] = type;
}

inline bool chunk_rendering_t::is_ready_to_render() const {
    return ready_to_render;
}

inline bool chunk_rendering_t::is_rendering_enabled() const {
    return rendering_enabled_info;
}

#endif
