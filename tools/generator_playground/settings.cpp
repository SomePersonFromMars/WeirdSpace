#include "useful.hpp"
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
#ifdef DEBUG
		READ_FIELD( enable_breakpoints )
#endif

	}
#undef READ_FIELD

	file.close();
}
