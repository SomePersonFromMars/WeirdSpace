
find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# World generator playground
add_executable(generator_playground
	generator_playground.cpp
	app.cpp
	app_imgui.cpp
	app_generator.cpp
	settings.cpp
	useful.cpp
	geometry.cpp
	shader_loader.cpp
	bitmap.cpp
	line.cpp

	generatorC.cpp
	generatorC_tour.cpp
	generatorC_GPU.cpp
	noise.cpp
	voronoi.cpp
	)
target_link_libraries(generator_playground
	${ALL_LIBS}
	)
