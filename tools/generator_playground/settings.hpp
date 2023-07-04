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

#define SHADER_GENERATOR_D_COMPUTE_PATH \
	"runtime/shader_generator_D_compute.glsl"

#define SHADER_GENERATOR_D_VERRTEX_PATH \
	"runtime/shader_generator_D_vertex.glsl"
#define SHADER_GENERATOR_D_FRAGMENT_PATH \
	"runtime/shader_generator_D_fragment.glsl"

#include <cstddef>
#include <cstdint>
#include <climits>

extern struct settings_t {
#define FIELD(T, name, val, min, max) \
	T name = val; \
	static constexpr T name##_min = min; \
	static constexpr T name##_max = max;

	FIELD(std::size_t, voro_cnt         , 240,      3,    240*2)
	FIELD(std::size_t, super_voro_cnt   , 80,       3,    240*2)
	FIELD(bool       , draw_mid_polygons, false,    0,    1)
	FIELD(std::size_t, replace_seed     , 0,        0,    ULLONG_MAX)
	FIELD(double     , river_joints_R   , 0.015,    0.0,  1.0)
	FIELD(int        , river_start_prob , 20,       0,    100)
	FIELD(int        , river_branch_prob, 20,       0,    100)
	FIELD(uint32_t   , river_color      , 0x477199, 0,    0xffffff)
	FIELD(int        , humidity_scale   , 5,        0,    200)
	FIELD(double     , temperature_exp  , 4.0,      0.01, 100.0)
	// Chunk dimension in the actual game is 512.
	// This constant only changes the chunk resolution
	// and not its scale compared to objects dimensions.
	// For example noise features remain the same
	// size compared to the chunk dimensions.
	FIELD(int        , chunk_dim        , 64,       1,    4096)
	FIELD(bool       , generate_with_gpu, false,    0,    1)
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
