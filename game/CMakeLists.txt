
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(${CUSTOM_DEBUG_FLAGS})
endif()

# Weird Space
add_custom_target(link_game_runtime_dir ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_SOURCE_DIR}/game/runtime" "${CMAKE_BINARY_DIR}/game/runtime")

include_directories(
    .
)

add_executable(weird_space
    common_shading/shader_A.cpp

    root/app.cpp
    root/app_imgui.cpp
    root/callbacks.cpp
    root/weird_space.cpp

    settings/global_settings_gui.cpp
    settings/settings.cpp

    camera/camera.cpp

    player/player.cpp

    utilities/math/bounding_volume.cpp
    utilities/math/geometry.cpp

    utilities/expiration_queue.cpp
    utilities/imgui_basic_controls.cpp
    utilities/shader_loader.cpp
    utilities/texture_loader.cpp
    utilities/useful.cpp

    procedural_map/map_generator.cpp
    procedural_map/map_generator_cpu_drawing_helpers.cpp
    procedural_map/map_generator_GPU.cpp
    procedural_map/map_generator_tour.cpp
    procedural_map/map_storage.cpp
    procedural_map/noise.cpp
    procedural_map/voronoi.cpp

    world/chunk_content.cpp
    world/chunk_rendering.cpp
    world/resources_manager_world.cpp
    world/shader_world.cpp
    world/world_buffer.cpp
    world/world_generator.cpp
)
target_link_libraries(weird_space
    ${ALL_LIBS}
)

add_dependencies(weird_space link_game_runtime_dir)
