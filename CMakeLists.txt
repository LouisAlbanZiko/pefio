cmake_minimum_required(VERSION 3.24.0)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_out/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_out/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_out/bin)

	project(c_file)

	add_subdirectory(test)
endif()

if( C_FILE_INCLUDE_DEPENDENCIES )
	add_subdirectory(_vendor/c_core)
endif()

add_subdirectory(src)
