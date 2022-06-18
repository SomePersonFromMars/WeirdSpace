# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Spcol)

find_package(OpenGL REQUIRED)


# Compile external dependencies
add_subdirectory (external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	# playground/
	)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
	)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
	)

add_subdirectory(playground)
