set(UTILS_PATH
	${CMAKE_SOURCE_DIR}/game/utils)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(${CUSTOM_DEBUG_FLAGS})
endif()

# Obj_dump
add_executable(obj_dump
	obj_dump.cpp
	objloader.cpp
	)
target_link_libraries(obj_dump
	${ALL_LIBS}
	)
