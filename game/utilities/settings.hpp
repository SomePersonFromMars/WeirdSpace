#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

// Minimal frame duration in ms
// #define FRAME_MIN_DURATION 1
#define FRAME_MIN_DURATION 16 // 60fps
// #define FRAME_MIN_DURATION 33 // 30fps
// #define FRAME_MIN_DURATION 46 // 20fps
// #define FRAME_MIN_DURATION 120

#define SHADER_A_VERTEX_PATH "runtime/shader_A_vertex.glsl"
#define SHADER_A_FRAGMENT_PATH "runtime/shader_A_fragment.glsl"
#define SHADER_WORLD_VERTEX_PATH "runtime/shader_world_vertex.glsl"
// #define TEXTURE_BLOCKS_COMBINED_PATH "runtime/blocks_combined.png"
#define TEXTURE_BLOCKS_COMBINED_PATH "runtime/blocks_combined-packed.png"
// #define TEXTURE_BLOCKS_COMBINED_PATH "runtime/debug.png"
// #define TEXTURE_BLOCKS_COMBINED_PATH "runtime/sand.png"
#define TEXTURE_PLAYER_PATH "runtime/player.png"

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

    // World rendering and preprocessing
	FIELD(float      , font_global_scale          , 1.5f,     1.0f, 2.0f)
	FIELD(uint32_t   , sky_color                  , 0xfccc92, 0,    0xffffff)
	FIELD(uint32_t   , light_color                , 0xf7d5ad, 0,    0xffffff)
	FIELD(float      , render_distance            , 8.0f,     1.0f, 32.0f)
	FIELD(std::size_t, max_preprocessed_chunks_cnt, 25,       1,    10'000)

    // World shape and world experience
	FIELD(int        , terrain_height_in_blocks   , 64,        1,    128)
    float default_player_position[3]              {200.0f, 128.0f, 231.5f};

    // On top of map and map experience
	FIELD(bool       , draw_player                , true,    0,    1)
	FIELD(bool       , draw_mid_polygons          , false,    0,    1)
	FIELD(bool       , dynamic_map                , false,    0,    1)

    // Map rendering
	FIELD(bool       , generate_with_gpu          , true,     0,    1)
	FIELD(bool       , triple_map_size            , false,    0,    1)

    // Map shape
	FIELD(std::size_t, replace_seed               , 0,        0,    ULLONG_MAX)
	FIELD(std::size_t, voro_cnt                   , 402,      3,    240*2)
	FIELD(std::size_t, super_voro_cnt             , 22,       3,    240*2)
    FIELD(float      , land_probability           , 0.8f,     0.0f, 1.0f)
	FIELD(int        , map_unit_resolution        , 512,      1,    4096)
	FIELD(int        , map_width_in_units         , 4,        1,    4096)
	FIELD(int        , map_height_in_units        , 2,        1,    4096)

    // Rivers and climate
	FIELD(bool       , generate_rivers            , true,     0,    1)
	FIELD(double     , river_joints_R             , 0.0106,   0.0,  1.0)
	FIELD(int        , river_start_prob           , 12,       0,    100)
	FIELD(int        , river_branch_prob          , 20,       0,    100)
	FIELD(uint32_t   , river_color                , 0x477199, 0,    0xffffff)
	FIELD(bool       , draw_temperature           , false,    0,    1)
	FIELD(bool       , draw_humidity              , true,     0,    1)
	FIELD(int        , humidity_scale             , 5,        0,    200)
	FIELD(double     , temperature_exp            , 4.0,      0.01, 100.0)

    // Others
	size_t debug_vals[3]                          {7, 0, 0};
#undef FIELD

	static constexpr char settings_file_path[] = "runtime/settings.txt";

	void save_settings_to_file();
	void load_settings_from_file(const char * const path = settings_file_path);

    inline void request_global_reload();
    inline bool is_global_reload_pending() const;
    inline void mark_global_reload_completed();

    inline void request_possibly_no_restart_reload();
    inline bool is_possibly_no_restart_reload_pending() const;
    inline void mark_possibly_no_restart_reload_completed();

    inline void request_replace_seed_overwrite();
    inline void supply_new_replace_seed(std::size_t new_replace_seed);

private:
    bool global_reload_requested = false;
    bool possibly_no_restart_reload_requested = false;
    bool replace_seed_overwrite_requested = false;
} global_settings;

inline void settings_t::request_global_reload() {
    global_reload_requested = true;
}
inline void settings_t::mark_global_reload_completed() {
    global_reload_requested = false;
}
inline bool settings_t::is_global_reload_pending() const {
    return global_reload_requested;
}

inline void settings_t::request_possibly_no_restart_reload() {
    possibly_no_restart_reload_requested = true;
}
inline void settings_t::mark_possibly_no_restart_reload_completed() {
    possibly_no_restart_reload_requested = false;
}
inline bool settings_t::is_possibly_no_restart_reload_pending() const {
    return possibly_no_restart_reload_requested;
}

inline void settings_t::request_replace_seed_overwrite() {
    replace_seed_overwrite_requested = true;
}
inline void settings_t::supply_new_replace_seed(std::size_t new_replace_seed) {
    if (replace_seed_overwrite_requested) {
        replace_seed_overwrite_requested = false;
        replace_seed = new_replace_seed;
    }
}

#endif
