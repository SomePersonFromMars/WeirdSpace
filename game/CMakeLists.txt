
find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Weird Space
add_executable(weird_space
	weird_space.cpp
	app.cpp
	app_imgui.cpp

	callbacks.cpp
	camera.cpp
	bounding_volume.cpp
	chunk.cpp
	world_buffer.cpp
	world_generator.cpp
	player.cpp
	shader_A.cpp
	shader_world.cpp

	map_generator/noise.cpp
	map_generator/voronoi.cpp
	map_generator/map_storage.cpp
	map_generator/map_generator.cpp
	map_generator/map_generator_cpu_drawing_helpers.cpp
	map_generator/map_generator_GPU.cpp
	map_generator/map_generator_tour.cpp

	utilities/settings.cpp
	utilities/useful.cpp
	utilities/imgui_basic_controls.cpp
	utilities/global_settings_gui.cpp
	utilities/geometry.cpp
	utilities/texture_loader.cpp
	utilities/shader_loader.cpp
	)
target_link_libraries(weird_space
	${ALL_LIBS}
	)
