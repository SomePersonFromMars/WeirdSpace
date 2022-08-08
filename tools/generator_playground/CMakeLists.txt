
# World generator playground
add_executable(generator_playground
	generator_playground.cpp
	useful.cpp
	shader_loader.cpp
	bitmap.cpp

	generator.cpp
	noise.cpp
	)
target_link_libraries(generator_playground
	${ALL_LIBS}
	)
