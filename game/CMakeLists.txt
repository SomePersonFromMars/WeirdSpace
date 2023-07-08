
# Weird Space
add_executable(weird_space
	weird_space.cpp
	app.cpp

	callbacks.cpp
	camera.cpp
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
	utilities/geometry.cpp
	utilities/texture_loader.cpp
	utilities/shader_loader.cpp
	)
target_link_libraries(weird_space
	${ALL_LIBS}
	)
