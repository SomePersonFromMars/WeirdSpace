#include "app.hpp"
#include "shader_A.hpp"

#include <chrono>
#include <cstdlib>
#include <thread>

#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <useful.hpp>
#include <settings.hpp>

// App
app_t::app_t()
	:world_generator(
			map_storage,
			world_buffer)
	,player(shader_A, world_buffer)
	,camera(
            glm::vec3(
                // 0.0, 100.0, 0.0
                902, 126, 77
                ),
			5.851774, 5.900709,
			90.0f,
			0.1f
			// static_cast<float>(chunk_t::WIDTH) * 16
			)
	,map_generator(&map_storage)
	,callbacks_strct(
			window_width,
			window_height,
			delta_time,
			camera,
			player)
{ }

// Init
void app_t::init() {
	global_settings.load_settings_from_file();

	init_opengl_etc();
	init_callbacks();
	init_imgui();

	init_map_related();
	init_world_blocks();

	init_camera();

	init_player();
}

// Loop
void app_t::loop() {
    auto timer_fps_cnter = std::chrono::high_resolution_clock::now();

	while (glfwWindowShouldClose(window) == GLFW_FALSE) {
		constexpr auto frame_min_duration
			= std::chrono::milliseconds(FRAME_MIN_DURATION);

		const auto frame_beg_time = std::chrono::high_resolution_clock::now();
		const auto frame_end_time
			= frame_beg_time + frame_min_duration;

        const glm::vec3 background_color = color_hex_to_vec3(global_settings.sky_color);
		glClearColor(background_color.x, background_color.y, background_color.z,
				0.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, window_width, window_height);

        camera.load_settings();
		const glm::mat4 projection_matrix
			= camera.calculate_projection_matrix(get_window_aspect_ratio());
		const glm::mat4 view_matrix = camera.calculate_view_matrix();
		const frustum_t camera_frustum
			= camera.calculate_frustum_planes(get_window_aspect_ratio());

		// const glm::vec3 light_pos
		// 	= camera.get_position() + glm::vec3(0, 5, 0);

        const shader_A_fragment_common_uniforms_t shader_A_fragment_common_uniforms {
            // camera_pos_worldspace:
            camera.get_position(),

            // light_pos_worldspace:
            // camera.get_position() + glm::vec3(0, 5, 0),
            // glm::vec3(0, chunk_t::HEIGHT, world_buffer.get_world_depth()),
            camera.get_position() + glm::vec3(8, 5, 5),

            // sun_direction_worldspace:
            glm::normalize(-glm::vec3(-15, 5, 10)),

            // light_color:
            color_hex_to_vec3(global_settings.light_color),

            // fog_color:
            color_hex_to_vec3(global_settings.sky_color),
        };

		for (auto &[buffer_pos_XZ, chunk] : world_buffer.chunks) {
			const glm::vec3 buffer_pos_XYZ = {
				static_cast<float>(buffer_pos_XZ.x),
				0.0f,
				static_cast<float>(buffer_pos_XZ.y)
			};
            const float world_buffer_width = world_buffer.get_world_width();

            chunk.calculate_preprocessing_priority(
                    buffer_pos_XYZ,
                    world_buffer_width,
                    camera);

			chunk.draw_cyclicly_if_visible(
				projection_matrix,
				view_matrix,
				shader_A_fragment_common_uniforms,
				buffer_pos_XYZ,
                world_buffer_width,
				camera_frustum);
		}

		player.draw_cyclic(projection_matrix, view_matrix, shader_A_fragment_common_uniforms);

		in_loop_update_imgui();

        const auto now = std::chrono::high_resolution_clock::now();
        delta_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - timer_fps_cnter).count()
            / 1000.0;
        timer_fps_cnter = now;

        global_settings.supply_new_replace_seed(map_generator.get_current_voronoi_seed());

        if (
            global_settings.is_global_reload_pending() or
            global_settings.is_possibly_no_restart_reload_pending()
            ) {
            global_settings.mark_possibly_no_restart_reload_completed();
            global_settings.request_global_reload();
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

		glfwSwapBuffers(window);

		glfwPollEvents();
		callbacks_strct.handle_input();

		player.update_physics(delta_time);

		if (camera.get_following_mode())
			camera.follow(delta_time,
					player.get_position() + glm::vec3(0.5, 1, 0));

		std::this_thread::sleep_until(frame_end_time);
	}
}

// Deinit
void app_t::deinit() {
	deinit_imgui();
	deinit_world_blocks();
	deinit_player();
	deinit_map_related();
	deinit_opengl_etc();

	global_settings.save_settings_to_file();
}


// Init subfunctions
void app_t::init_opengl_etc() {
	if( !glfwInit() ) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow( window_width, window_height, "Playground",
			nullptr, nullptr);
	if( window == nullptr ){
		fprintf(stderr,
			"Failed to open GLFW window. If you have an Intel GPU, they"
			"are not 3.3 compatible.\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}
	glGetError();
}

void app_t::init_camera() {
    camera.load_settings();
    camera.init_cyclicness(world_buffer.get_world_width());
	// camera.switch_following_mode();
}

void app_t::init_map_related() {
	map_storage.load_settings();
	map_storage.init_gl();
	map_storage.reallocate_gpu_and_cpu_memory();

	map_generator.load_settings();
	map_generator.init_gl();

	map_generator.new_seed();
	map_generator.generate_map();

	// map_storage.load_from_cpu_to_gpu_memory();
	// map_storage.clear();
	map_storage.load_from_gpu_to_cpu_memory();
	PRINT_D((int)map_storage.get_component_value(0, 0, 0));
}

void app_t::init_world_blocks() {
	shader_A.init();
	shader_world.init();

	chunk_t::init_gl_static(&shader_world);

    world_buffer.load_settings();
	const int CHUNKS_X_CNT = world_buffer.get_buffer_width();
	const int CHUNKS_Z_CNT = world_buffer.get_buffer_depth();
    world_generator.load_settings();
	for (int x = 0; x < CHUNKS_X_CNT; ++x) {
		for (int z = 0; z < CHUNKS_Z_CNT; ++z) {
			world_generator.gen_chunk({x, z});
		}
	}

	for (int x = 0; x < CHUNKS_X_CNT; ++x) {
		for (int z = 0; z < CHUNKS_Z_CNT; ++z) {
			chunk_t &chunk = world_buffer.chunks[glm::ivec2(x, z)];
			if (x > 0)
				chunk.neighbors[0] = &world_buffer.chunks[glm::ivec2(x-1, z)];
			else if (x == 0)
				chunk.neighbors[0] = &world_buffer.chunks[glm::ivec2(
						world_buffer.get_buffer_width()-1, z)];
			if (x < CHUNKS_X_CNT-1)
				chunk.neighbors[1] = &world_buffer.chunks[glm::ivec2(x+1, z)];
			else if (x == CHUNKS_X_CNT-1)
				chunk.neighbors[1] = &world_buffer.chunks[glm::ivec2(0, z)];

			if (z > 0)
				chunk.neighbors[4] = &world_buffer.chunks[glm::ivec2(x, z-1)];
			if (z < CHUNKS_Z_CNT-1)
				chunk.neighbors[5] = &world_buffer.chunks[glm::ivec2(x, z+1)];

			chunk.preprocess_on_cpu();
			chunk.send_preprocessed_to_gpu();
		}
	}
}

void app_t::init_player() {
	player.init_gl();

	// player.debug_position = {chunk_t::WIDTH/2.0, chunk_t::HEIGHT, 0.5};
	// player.debug_position = {chunk_t::WIDTH/2.0, chunk_t::HEIGHT,
	// 	float(chunk_t::DEPTH/2)+0.5};
	// player.debug_position = {chunk_t::WIDTH/2.0 + chunk_t::WIDTH*3, chunk_t::HEIGHT,
	// 	float(chunk_t::DEPTH + world_buffer.get_world_depth())/2.0f + 0.5};
    // player.debug_position = {200.0, chunk_t::HEIGHT, 231.5};
	// player.debug_position = {0, chunk_t::HEIGHT, 0.5};
    player.debug_position[0] = global_settings.default_player_position[0];
    player.debug_position[1] = global_settings.default_player_position[1];
    player.debug_position[2] = global_settings.default_player_position[2];
	player.set_position(player.debug_position);
}

void app_t::init_callbacks() {
	callbacks_strct.init_gl(window);
}


// Deinit subfunctions
void app_t::deinit_opengl_etc() {
	glfwDestroyWindow(window);
	glfwTerminate();
	GL_GET_ERROR;
}

void app_t::deinit_map_related() {
	map_storage.deinit_gl();
	map_generator.deinit_gl();
}

void app_t::deinit_world_blocks() {
	chunk_t::deinit_gl_static();
	shader_A.deinit();
	shader_world.deinit();
}

void app_t::deinit_player() {
	player.deinit_gl();
}
