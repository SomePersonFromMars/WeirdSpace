#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

// Minimal frame duration in ms
// #define FRAME_MIN_DURATION 1
// #define FRAME_MIN_DURATION 16 // 60fps
// #define FRAME_MIN_DURATION 33 // 30fps
#define FRAME_MIN_DURATION 46 // 20fps
// #define FRAME_MIN_DURATION 120

#define SHADER_BITMAP_VERTEX_PATH "runtime/shader_bitmap_vertex.glsl"
#define SHADER_BITMAP_FRAGMENT_PATH "runtime/shader_bitmap_fragment.glsl"

#define SHADER_LINE_VERTEX_PATH "runtime/shader_line_vertex.glsl"
#define SHADER_LINE_FRAGMENT_PATH "runtime/shader_line_fragment.glsl"

// Chunk dimension in the actual game is 512.
// This constant only changes the chunk resolution
// and not its scale compared to objects dimensions.
// For example noise features remain the same
// size compared to the chunk dimensions.
#define CHUNK_DIM 64

#include <cstddef>

struct settings_t {
};

#endif
