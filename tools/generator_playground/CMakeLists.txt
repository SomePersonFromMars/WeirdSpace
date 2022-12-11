
# World generator playground
add_executable(generator_playground
	generator_playground.cpp
	useful.cpp
	shader_loader.cpp
	bitmap.cpp
	line.cpp

	generator.cpp
	noise.cpp
	voronoi.cpp
	)
target_link_libraries(generator_playground
	${ALL_LIBS}
	)
