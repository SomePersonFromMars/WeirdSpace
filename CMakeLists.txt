# CMake entry point
cmake_minimum_required (VERSION 3.20)
project (WeirdSpace)

set (CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)


# Compile external dependencies
add_subdirectory (external)

include_directories(
	external/glfw-3.3.7/include/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	external/stb/include/
	external/perlin_noise/include/
	external/delaunator-cpp/include/
	external/imgui/
	external/imgui/backends/
	)

include_directories(
	game/utilities/
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
	delaunator-cpp
	imgui
	)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
	)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1 CACHE BOOL "Enable/Disable output of compile commands during generation." FORCE)

set(CUSTOM_DEBUG_FLAGS
    -DDEBUG
    -Wall
    # -Wshadow
    -Wextra
    -Wpedantic
    -pedantic)

add_subdirectory(game)
add_subdirectory(tools/obj_dump)
add_subdirectory(tools/generator_playground)
