// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#include "app.hpp"
#include "chunk.hpp"
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
                 902, 126, 77
                 // 179, 21, 183
                 ),
             5.851774, 5.900709,
             // 5.851774, 0,
             90.0f,
             0.1f
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

        const float aspect = get_window_aspect_ratio();
        camera.load_settings(aspect);
        const glm::mat4 projection_matrix
            = camera.calculate_projection_matrix(aspect);
        const glm::mat4 view_matrix = camera.calculate_view_matrix();
        const frustum_t camera_frustum
            = camera.calculate_frustum_planes(aspect);

        const shader_A_fragment_common_uniforms_t shader_A_fragment_common_uniforms {
            camera.get_position(), // camera_pos_worldspace
                camera.get_position() + glm::vec3(8, 5, 5), // light_pos_worldspace
                glm::normalize(-glm::vec3(-15, 5, 10)), // sun_direction_worldspace
                color_hex_to_vec3(global_settings.light_color), // light_color
                color_hex_to_vec3(global_settings.sky_color), // fog_color
        };

        // FIXME: Incorrect chunks settings loading on soft reload makes the game crush.
        if (global_settings.are_chunks_rendering_settings_updated()) {
            world_buffer.preprocessing_system_load_settings_reload();
            global_settings.mark_chunks_rendering_settings_applied();
        }
        world_buffer.refresh_preprocessing_tasks(camera);
        world_buffer.preprocess_most_urgent_chunk();
        // while (world_buffer.preprocess_most_urgent_chunk()) // TODO: Process tasks in another thread.
        //     ;
        world_buffer.draw_cyclicly_if_visible(
                projection_matrix,
                view_matrix,
                shader_A_fragment_common_uniforms,
                camera_frustum
                );

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

    window = glfwCreateWindow( window_width, window_height, "WeirdSpace",
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
    camera.load_settings(get_window_aspect_ratio());
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

    map_storage.load_from_gpu_to_cpu_memory();
    PRINT_D((int)map_storage.get_component_value(0, 0, 0));
}

void app_t::init_world_blocks() {
    shader_A.init();
    shader_world.init();

    chunk_rendering_t::init_gl_static(&shader_world);

    world_buffer.load_settings();
    world_buffer.preprocessing_system_load_settings_reload();
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
            chunk_content_t &chunk_content = world_buffer.get_chunk_content(glm::ivec2(x, z));

            if (x > 0)                      chunk_content.neighbors[0] = &world_buffer.get_chunk_content(glm::ivec2(x-1, z));
            else if (x == 0)                chunk_content.neighbors[0] = &world_buffer.get_chunk_content(glm::ivec2(world_buffer.get_buffer_width()-1, z));
            if (x < CHUNKS_X_CNT-1)         chunk_content.neighbors[1] = &world_buffer.get_chunk_content(glm::ivec2(x+1, z));
            else if (x == CHUNKS_X_CNT-1)   chunk_content.neighbors[1] = &world_buffer.get_chunk_content(glm::ivec2(0, z));

            if (z > 0)                      chunk_content.neighbors[4] = &world_buffer.get_chunk_content(glm::ivec2(x, z-1));
            if (z < CHUNKS_Z_CNT-1)         chunk_content.neighbors[5] = &world_buffer.get_chunk_content(glm::ivec2(x, z+1));
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
    chunk_rendering_t::deinit_gl_static();
    shader_A.deinit();
    shader_world.deinit();
}

void app_t::deinit_player() {
    player.deinit_gl();
}
