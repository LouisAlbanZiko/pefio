#ifndef _C_FILE_INTERNAL_H_
#define _C_FILE_INTERNAL_H_

#include <c_core/c_core.h>
#include <c_file/c_file.h>

#ifdef __unix__
	
#elif defined _WIN32 || defined WIN32
#include <windows.h>
#else
#ERROR platform not implemented
#endif

#ifdef __unix__

#elif defined _WIN32 || defined WIN32

typedef struct CF_File
{
	char *path;
	HANDLE handle_file;
	HANDLE handle_mapping;
	uint64_t size;
	void *data;
	CC_UnorderedSet *views;
} CF_File;

typedef struct CF_FileView
{
	CF_File *file;
	uint64_t start;
	uint64_t size;
	uint8_t *data;
} CF_FileView;

#endif

// error
void _cf_make_error(uint64_t error_type, const char *format, ...);

#endif