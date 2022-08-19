cmake_minimum_required(VERSION 3.24.0)

file(GLOB_RECURSE SRC_FILES ./*.c)
add_library(c_file STATIC ${SRC_FILES})

target_include_directories(c_file
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(c_file c_core)
