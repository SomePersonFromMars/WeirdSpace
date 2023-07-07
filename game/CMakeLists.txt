
# Weird Space
add_executable(weird_space
	weird_space.cpp
	callbacks.cpp
	camera.cpp
	chunk.cpp
	world_buffer.cpp
	world_generator.cpp
	player.cpp
	shader_A.cpp
	shader_world.cpp

	map_generator/noise.cpp

	utilities/useful.cpp
	utilities/texture_loader.cpp
	utilities/shader_loader.cpp
	)
target_link_libraries(weird_space
	${ALL_LIBS}
	)
