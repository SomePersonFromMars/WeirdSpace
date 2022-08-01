
# Weird Space
add_executable(weird_space
	weird_space.cpp
	useful.cpp
	texture_loader.cpp
	callbacks.cpp
	camera.cpp
	chunk.cpp
	noise.cpp
	world_buffer.cpp
	world_generator.cpp
	player.cpp
	shader_loader.cpp
	shader_A.cpp
	shader_world.cpp
	)
target_link_libraries(weird_space
	${ALL_LIBS}
	)
