set(UTILS_PATH
	${CMAKE_SOURCE_DIR}/game/utils)

# Obj_dump
include_directories(
	.
	${UTILS_PATH}
	)

add_executable(obj_dump
	obj_dump.cpp
	${UTILS_PATH}/objloader.cpp
	)
target_link_libraries(obj_dump
	${ALL_LIBS}
	)
