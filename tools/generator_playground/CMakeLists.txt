
find_package(OpenMP)
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Map generator playground
add_executable(generator_playground
	generator_playground.cpp
	app.cpp
	app_imgui.cpp
	app_generator.cpp
	line.cpp

	map_generator/noise.cpp
	map_generator/voronoi.cpp
	map_generator/map_storage.cpp
	map_generator/map_generator.cpp
	map_generator/map_generator_cpu_drawing_helpers.cpp
	map_generator/map_generator_tour.cpp
	map_generator/map_generator_GPU.cpp

	utilities/settings.cpp
	utilities/useful.cpp
	utilities/imgui_basic_controls.cpp
	utilities/global_settings_gui.cpp
	utilities/geometry.cpp
	utilities/shader_loader.cpp
	)
target_link_libraries(generator_playground
	${ALL_LIBS}
	)
