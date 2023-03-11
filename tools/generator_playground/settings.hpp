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
#include <climits>

extern struct settings_t {
#define FIELD(T, name, val, min, max) \
	T name = val; \
	static constexpr T name##_min = min; \
	static constexpr T name##_max = max;

	FIELD(std::size_t, voro_cnt          , 240, 3, 240*2)
	FIELD(std::size_t, super_voro_cnt    , 80,  3, 240*2)
	FIELD(bool,        draw_mid_polygons , false, 0, 1)
	FIELD(std::size_t, replace_seed      , 0,   0, ULLONG_MAX)
	size_t debug_vals[3] {
		7,
		0,
		0
	};

#undef FIELD

	static constexpr char settings_file_path[] = "runtime/settings.txt";
	void save_settings_to_file();
	void load_settings_from_file();
} global_settings;

#endif
