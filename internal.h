#ifndef _C_FILE_INTERNAL_H_
#define _C_FILE_INTERNAL_H_

#include "c_file.h"

#ifdef __unix__
	
#elif defined _WIN32 || defined WIN32
#include <windows.h>
#else
#ERROR platform not found
#endif

#ifdef __unix__

#elif defined _WIN32 || defined WIN32
typedef struct CF_File
{
	CC_String path;
	HANDLE handle_file;
	uint64_t size;
	CC_UnorderedSet *file_views;
} CF_File;

typedef struct CF_FileView
{
	CF_File *file;
	HANDLE handle_mapping;
	uint64_t start;
	uint64_t size;
	uint8_t *data;
} CF_FileView;

typedef struct CF_TableFile
{
	CC_String path;
	HANDLE file_handle;
	HANDLE file_mapping;
} CF_TableFile;
#endif

// error
void _cf_make_error(uint64_t error_type, const char *format, ...);

#endif