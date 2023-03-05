
# World generator playground
add_executable(generator_playground
	generator_playground.cpp
	settings.cpp
	useful.cpp
	geometry.cpp
	shader_loader.cpp
	bitmap.cpp
	line.cpp

	generatorAB.cpp
	generatorC.cpp
	generatorC_tour.cpp
	noise.cpp
	voronoi.cpp
	)
target_link_libraries(generator_playground
	${ALL_LIBS}
	)
