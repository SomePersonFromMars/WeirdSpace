set(UTILS_PATH
	${CMAKE_SOURCE_DIR}/game/utils)

# Obj_dump
add_executable(obj_dump
	obj_dump.cpp
	objloader.cpp
	)
target_link_libraries(obj_dump
	${ALL_LIBS}
	)
