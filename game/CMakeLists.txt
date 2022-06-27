
# Weird Space
include_directories(
	.
	)

file(GLOB UTILS_SRC utils/*.cpp)

add_executable(weird_space
	weird_space.cpp
	chunk.cpp
	world_generator.cpp
	# shader.cpp
	${UTILS_SRC}
	)
target_link_libraries(weird_space
	${ALL_LIBS}
	)
