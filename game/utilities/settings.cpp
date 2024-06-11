// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include <useful.hpp>
#include "settings.hpp"

#include <cstdio>
#include <fstream>
#include <cassert>

settings_t global_settings;

void settings_t::save_settings_to_file() {
	std::ofstream file(settings_file_path);
	fprintf(stderr,
		"Saving settings to %s file.\n", settings_file_path);
	assert(file.good());

#define WRITE_FIELD(name) \
	file << #name << " " << name << '\n';

	WRITE_FIELD( font_global_scale )
	WRITE_FIELD( sky_color )
	WRITE_FIELD( light_color )
	WRITE_FIELD( render_distance )
	WRITE_FIELD( camera_rotation_speed_normal )
	WRITE_FIELD( camera_moving_speed_normal )
	WRITE_FIELD( max_preprocessed_chunks_cnt )
    file << '\n';

	WRITE_FIELD( terrain_height_in_blocks )
	WRITE_FIELD( default_player_position[0] )
	WRITE_FIELD( default_player_position[1] )
	WRITE_FIELD( default_player_position[2] )
    file << '\n';

	WRITE_FIELD( draw_player )
	WRITE_FIELD( draw_mid_polygons )
	WRITE_FIELD( dynamic_map )
    file << '\n';

	WRITE_FIELD( generate_with_gpu )
	WRITE_FIELD( triple_map_size )
    file << '\n';

	WRITE_FIELD( replace_seed )
	WRITE_FIELD( voro_cnt       )
	WRITE_FIELD( super_voro_cnt )
	WRITE_FIELD( land_probability )
	WRITE_FIELD( map_unit_resolution )
	WRITE_FIELD( map_width_in_units )
	WRITE_FIELD( map_height_in_units )
    file << '\n';

	WRITE_FIELD( generate_rivers )
	WRITE_FIELD( river_joints_R )
	WRITE_FIELD( river_start_prob )
	WRITE_FIELD( river_branch_prob )
	WRITE_FIELD( river_color )
	WRITE_FIELD( draw_temperature )
	WRITE_FIELD( draw_humidity )
	WRITE_FIELD( humidity_scale )
	WRITE_FIELD( temperature_exp )
    file << '\n';

	WRITE_FIELD( enable_breakpoints )
	WRITE_FIELD( debug_vals[0] )
	WRITE_FIELD( debug_vals[1] )
	WRITE_FIELD( debug_vals[2] )

#undef WRITE_FIELD

	file.close();
}

void settings_t::load_settings_from_file(const char * const path) {
	std::ifstream file(path);
	fprintf(stderr,
		"Loading settings from %s file.\n", path);
	if (!file.good()) {
		fprintf(stderr,
			"%s settings file does not exist.\n"
			"Using previous (default) settings instead.\n",
			path);
		return;
	}

#define READ_FIELD(name) \
	if (key == #name) file >> name;
	std::string key;
	while (file >> key) {

        READ_FIELD( font_global_scale )
        READ_FIELD( sky_color )
        READ_FIELD( light_color )
        READ_FIELD( render_distance )
        READ_FIELD( camera_rotation_speed_normal )
        READ_FIELD( camera_moving_speed_normal )
        READ_FIELD( max_preprocessed_chunks_cnt )

        READ_FIELD( terrain_height_in_blocks )
        READ_FIELD( default_player_position[0] )
        READ_FIELD( default_player_position[1] )
        READ_FIELD( default_player_position[2] )

        READ_FIELD( draw_player )
        READ_FIELD( draw_mid_polygons )
        READ_FIELD( dynamic_map )

        READ_FIELD( generate_with_gpu )
        READ_FIELD( triple_map_size )

        READ_FIELD( replace_seed )
        READ_FIELD( voro_cnt       )
        READ_FIELD( super_voro_cnt )
        READ_FIELD( land_probability )
        READ_FIELD( map_unit_resolution )
        READ_FIELD( map_width_in_units )
        READ_FIELD( map_height_in_units )

        READ_FIELD( generate_rivers )
        READ_FIELD( river_joints_R )
        READ_FIELD( river_start_prob )
        READ_FIELD( river_branch_prob )
        READ_FIELD( river_color )
        READ_FIELD( draw_temperature )
        READ_FIELD( draw_humidity )
        READ_FIELD( humidity_scale )
        READ_FIELD( temperature_exp )

        READ_FIELD( enable_breakpoints )
        READ_FIELD( debug_vals[0] )
        READ_FIELD( debug_vals[1] )
        READ_FIELD( debug_vals[2] )

	}
#undef READ_FIELD

	file.close();
}
