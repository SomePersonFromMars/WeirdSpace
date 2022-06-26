
# Playground
include_directories(
	.
	)

file(GLOB UTILS_SRC utils/*.cpp)

add_executable(playground
	playground.cpp
	world.cpp
	# shader.cpp
	${UTILS_SRC}
	)
target_link_libraries(playground
	${ALL_LIBS}
	)
