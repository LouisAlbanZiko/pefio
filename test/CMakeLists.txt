cmake_minimum_required(VERSION 3.24.0)

file(GLOB_RECURSE SRC_FILES ./*.c)
add_executable(c_file_test ${SRC_FILES})

target_include_directories(c_file_test
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(c_file_test c_file)

set_target_properties(c_file_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
