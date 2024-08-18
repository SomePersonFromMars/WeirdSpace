
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(${CUSTOM_DEBUG_FLAGS})
endif()

# Map generator playground
add_custom_target(link_generator_playground_runtime_dir ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_SOURCE_DIR}/tools/generator_playground/runtime" "${CMAKE_BINARY_DIR}/tools/generator_playground/runtime")

add_executable(generator_playground
    generator_playground.cpp
    app.cpp
    app_imgui.cpp
    app_generator.cpp
    line.cpp

    procedural_map/noise.cpp
    procedural_map/voronoi.cpp
    procedural_map/map_storage.cpp
    procedural_map/map_generator.cpp
    procedural_map/map_generator_cpu_drawing_helpers.cpp
    procedural_map/map_generator_tour.cpp
    procedural_map/map_generator_GPU.cpp

    settings/settings.cpp
    settings/global_settings_gui.cpp

    utilities/math/geometry.cpp

    utilities/useful.cpp
    utilities/imgui_basic_controls.cpp
    utilities/shader_loader.cpp
)
target_link_libraries(generator_playground
    ${ALL_LIBS}
)

add_dependencies(generator_playground link_generator_playground_runtime_dir)
