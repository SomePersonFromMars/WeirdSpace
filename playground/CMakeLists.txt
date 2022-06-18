
# Playground
include_directories(
	.
	)

add_executable(playground
	playground.cpp
	shader.cpp
	)
target_link_libraries(playground
	${ALL_LIBS}
	)
