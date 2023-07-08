#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

// Minimal frame duration in ms
// #define FRAME_MIN_DURATION 1
// #define FRAME_MIN_DURATION 16 // 60fps
// #define FRAME_MIN_DURATION 33 // 30fps
#define FRAME_MIN_DURATION 46 // 20fps
// #define FRAME_MIN_DURATION 120

#define SHADER_A_VERTEX_PATH "runtime/shader_A_vertex.glsl"
#define SHADER_A_FRAGMENT_PATH "runtime/shader_A_fragment.glsl"
#define SHADER_WORLD_VERTEX_PATH "runtime/shader_world_vertex.glsl"
// #define TEXTURE_BLOCKS_COMBINED_PATH "runtime/blocks_combined.png"
#define TEXTURE_BLOCKS_COMBINED_PATH "runtime/blocks_combined-packed.png"
// #define TEXTURE_BLOCKS_COMBINED_PATH "runtime/debug.png"
// #define TEXTURE_BLOCKS_COMBINED_PATH "runtime/sand.png"
#define TEXTURE_PLAYER_PATH "runtime/player.png"

#define ATMOSPHERE 4
#if ATMOSPHERE == 1
	// Soft-blue, soft-yellow
	#define SKY_COLOR 0xd7e6e8
	#define LIGHT_COLOR 0xfce7b5
#endif
#if ATMOSPHERE == 2
	// Soft-blue, soft-blue
	// #define SKY_COLOR 0x00ccff // Blue
	// #define SKY_COLOR 0x9deff9
	#define SKY_COLOR 0xc9edf2
	// #define SKY_COLOR 0xd7e6e8 // Soft-blue

	// #define LIGHT_COLOR 0xeafcfa // Soft-blue
	// #define LIGHT_COLOR 0xedfcfa
	// #define LIGHT_COLOR 0xf4fcfb
	#define LIGHT_COLOR 0xffffff // White
#endif
#if ATMOSPHERE == 3
	// Orange sunset
	#define SKY_COLOR 0xfcb967
	#define LIGHT_COLOR 0xf7d5ad
#endif
#if ATMOSPHERE == 4
	// Soft orange desert
	#define SKY_COLOR 0xfccc92
	#define LIGHT_COLOR 0xf7d5ad
#endif
#if ATMOSPHERE == 5
	// Contrast
	#define SKY_COLOR 0x000000
	#define LIGHT_COLOR 0xffffff
#endif

#define SHADER_MAP_STORAGE_VERTEX_PATH "runtime/shader_map_vertex.glsl"
#define SHADER_MAP_STORAGE_FRAGMENT_PATH "runtime/shader_map_fragment.glsl"

#define SHADER_LINE_VERTEX_PATH "runtime/shader_line_vertex.glsl"
#define SHADER_LINE_FRAGMENT_PATH "runtime/shader_line_fragment.glsl"

#define SHADER_MAP_GENERATOR_COMPUTE_PATH \
	"runtime/shader_map_generator_compute.glsl"

#define SHADER_MAP_GENERATOR_VERRTEX_PATH \
	"runtime/shader_map_generator_vertex.glsl"
#define SHADER_MAP_GENERATOR_FRAGMENT_PATH \
	"runtime/shader_map_generator_fragment.glsl"

#include <cstddef>
#include <cstdint>
#include <climits>

extern struct settings_t {
#define FIELD(T, name, val, min, max) \
	T name = val; \
	static constexpr T name##_min = min; \
	static constexpr T name##_max = max;

	FIELD(std::size_t, voro_cnt          , 240,      3,    240*2)
	FIELD(std::size_t, super_voro_cnt    , 80,       3,    240*2)
	FIELD(bool       , draw_mid_polygons , false,    0,    1)
	FIELD(std::size_t, replace_seed      , 0,        0,    ULLONG_MAX)
	FIELD(double     , river_joints_R    , 0.015,    0.0,  1.0)
	FIELD(int        , river_start_prob  , 20,       0,    100)
	FIELD(int        , river_branch_prob , 20,       0,    100)
	FIELD(uint32_t   , river_color       , 0x477199, 0,    0xffffff)
	FIELD(bool       , generate_rivers   , false,    0,    1)
	FIELD(bool       , draw_temperature  , false,    0,    1)
	FIELD(bool       , draw_humidity     , false,    0,    1)
	FIELD(int        , humidity_scale    , 5,        0,    200)
	FIELD(double     , temperature_exp   , 4.0,      0.01, 100.0)
	// Chunk dimension in the actual game is 512.
	// This constant only changes the chunk resolution
	// and not its scale compared to objects dimensions.
	// For example noise features remain the same
	// size compared to the chunk dimensions.
	FIELD(int        , chunk_dim         , 256,      1,    4096)
	FIELD(bool       , generate_with_gpu , true,    0,    1)
	FIELD(bool       , triple_map_size   , false,    0,    1)
	FIELD(bool       , draw_player       , false,    0,    1)
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
