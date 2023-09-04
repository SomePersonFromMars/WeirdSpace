#include <useful.hpp>
#include "settings.hpp"

#include <cstdio>
#include <fstream>
#include <cassert>

settings_t global_settings;

void settings_t::save_settings_to_file() {
	std::ofstream file(settings_file_path);
	fprintf(stderr,
		"Saving settings %s file.\n", settings_file_path);
	assert(file.good());

#define WRITE_FIELD(name) \
	file << #name << " " << name << '\n';

	WRITE_FIELD( voro_cnt       )
	WRITE_FIELD( super_voro_cnt )
	WRITE_FIELD( debug_vals[0] )
	WRITE_FIELD( debug_vals[1] )
	WRITE_FIELD( debug_vals[2] )
	WRITE_FIELD( replace_seed )
	WRITE_FIELD( draw_mid_polygons )
	WRITE_FIELD( river_joints_R )
	WRITE_FIELD( river_start_prob )
	WRITE_FIELD( river_branch_prob )
	WRITE_FIELD( river_color )
	WRITE_FIELD( generate_rivers )
	WRITE_FIELD( draw_temperature )
	WRITE_FIELD( draw_humidity )
	WRITE_FIELD( humidity_scale )
	WRITE_FIELD( temperature_exp )
	WRITE_FIELD( map_unit_resolution )
	WRITE_FIELD( map_width_in_units )
	WRITE_FIELD( map_height_in_units )
	WRITE_FIELD( generate_with_gpu )
	WRITE_FIELD( triple_map_size )
	WRITE_FIELD( draw_player )
	WRITE_FIELD( max_preprocessed_chunks_cnt )
	WRITE_FIELD( font_global_scale )
#ifdef DEBUG
	WRITE_FIELD( enable_breakpoints )
#endif

#undef WRITE_FIELD

	file.close();
}

void settings_t::load_settings_from_file() {
	std::ifstream file(settings_file_path);
	fprintf(stderr,
		"Loading settings from %s file.\n", settings_file_path);
	if (!file.good()) {
		fprintf(stderr,
			"%s settings file does not exist.\n"
			"Using default settings instead.\n",
			settings_file_path);
		return;
	}

#define READ_FIELD(name) \
	if (key == #name) file >> name;
	std::string key;
	while (file >> key) {

		READ_FIELD( voro_cnt       )
		READ_FIELD( super_voro_cnt )
		READ_FIELD( debug_vals[0] )
		READ_FIELD( debug_vals[1] )
		READ_FIELD( debug_vals[2] )
		READ_FIELD( replace_seed )
		READ_FIELD( draw_mid_polygons )
		READ_FIELD( river_joints_R )
		READ_FIELD( river_start_prob )
		READ_FIELD( river_branch_prob )
		READ_FIELD( river_color )
		READ_FIELD( generate_rivers )
		READ_FIELD( draw_temperature )
		READ_FIELD( draw_humidity )
		READ_FIELD( humidity_scale )
		READ_FIELD( temperature_exp )
		READ_FIELD( map_unit_resolution )
		READ_FIELD( map_width_in_units )
		READ_FIELD( map_height_in_units )
		READ_FIELD( generate_with_gpu )
		READ_FIELD( triple_map_size )
		READ_FIELD( draw_player )
		READ_FIELD( max_preprocessed_chunks_cnt )
		READ_FIELD( font_global_scale )
#ifdef DEBUG
		READ_FIELD( enable_breakpoints )
#endif

	}
#undef READ_FIELD

	file.close();
}