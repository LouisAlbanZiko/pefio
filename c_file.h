#ifndef _C_FILE_H_
#define _C_FILE_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <c_mem/c_mem.h>

// directory

uint64_t cf_directory_create(CM_String path);
uint64_t cf_directory_destroy(CM_String path);

// file

uint64_t cf_file_create(CM_String path);
uint64_t cf_file_destroy(CM_String path);

typedef struct CF_File CF_File;

CF_File *cf_file_open(CM_String path);
uint64_t cf_file_close(CF_File *file);

uint64_t cf_file_resize(CF_File *file, uint64_t new_size);
uint64_t cf_file_size_get(CF_File *file);

// file view
typedef struct CF_FileView CF_FileView;

CF_FileView *cf_file_view_open(CF_File *file, uint64_t start, uint64_t size);
uint64_t cf_file_view_close(CF_FileView *view);

uint64_t cf_file_view_read(CF_FileView *view, uint64_t start, uint64_t size, void *data);
uint64_t cf_file_view_write(CF_FileView *view, uint64_t start, uint64_t size, const void *data);
uint64_t cf_file_view_flush(CF_FileView *view);
uint64_t cf_file_view_start_get(CF_FileView *view);
uint64_t cf_file_view_size_get(CF_FileView *view);

// error
typedef struct CF_Error
{
	uint64_t error_type;
	CM_String message;
} CF_Error;

typedef enum CF_ErrorType
{
	CF_ERROR_TYPE_NONE = 0,
	CF_ERROR_TYPE_NULL_DATA,
	CF_ERROR_TYPE_FILE_CREATION_FAILED,
	CF_ERROR_TYPE_FILE_DELETION_FAILED,
	CF_ERROR_TYPE_FILE_RESIZE_FAILED,
	CF_ERROR_TYPE_FILE_OPENING_FAILED,
	CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED,
	CF_ERROR_TYPE_FILE_NOT_FOUND
} CF_ErrorType;

CF_Error cf_error_message();



// table file
typedef struct CF_TableFile CF_TableFile;

CF_TableFile *cf_table_file_create();
void cf_table_file_destroy(CF_TableFile *file);

#endif