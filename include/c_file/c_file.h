#ifndef _C_FILE_H_
#define _C_FILE_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// directory

uint64_t CF_Directory_Exists(const char *path);

uint64_t CF_Directory_Create(const char *path);
uint64_t CF_Directory_Destroy(const char *path);

// file

uint64_t CF_File_Exists(const char *path);

uint64_t CF_File_Create(const char *path, uint64_t size);
uint64_t CF_File_Destroy(const char *path);

typedef struct CF_File CF_File;

CF_File *CF_File_Open(const char *path);
uint64_t CF_File_Close(CF_File *file);

uint64_t CF_File_Resize(CF_File *file, uint64_t new_size);
uint64_t CF_File_SizeGet(CF_File *file);

// file view
typedef struct CF_FileView CF_FileView;

CF_FileView *CF_FileView_Open(CF_File *file, uint64_t start, uint64_t size);
uint64_t CF_FileView_Close(CF_FileView *view);

uint64_t CF_FileView_Read(CF_FileView *view, uint64_t start, uint64_t size, void *data);
uint64_t CF_FileView_Write(CF_FileView *view, uint64_t start, uint64_t size, const void *data);
uint64_t CF_FileView_Flush(CF_FileView *view);
uint64_t CF_FileView_StartGet(CF_FileView *view);
uint64_t CF_FileView_SizeGet(CF_FileView *view);

// util
uint64_t CF_File_Load(CF_File *file, uint64_t size, void *data);
uint64_t CF_File_Dump(CF_File *file, uint64_t size, const void *data);

// error
typedef struct CF_Error
{
	uint64_t error_type;
	const char *message;
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

CF_Error cf_get_last_error();

#endif