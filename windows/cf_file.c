#if defined _WIN32 || defined WIN32

#include "internal.h"

uint64_t cf_file_create(CC_String path, uint64_t size)
{
	CC_String copy = cc_string_copy(path);

	HANDLE handle = CreateFileA(copy.data, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	cc_string_destroy(copy);
	
	if (handle == INVALID_HANDLE_VALUE)
	{
		uint64_t win32_error_code = GetLastError();
		_cf_make_error(CF_ERROR_TYPE_FILE_CREATION_FAILED, "Failed to create file: %s. win32 error %llu", copy.data, win32_error_code);
		return 0;
	}
	else
	{
		// resize file
		LARGE_INTEGER li_size = { .QuadPart = size };
		if (!SetFilePointerEx(handle, li_size, (LARGE_INTEGER *)&size, FILE_BEGIN))
		{
			uint64_t win32_error_code = GetLastError();
			_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set file pointer when extending file: %s. win32 error %llu", copy.data, win32_error_code);
			return 0;
		}

		if (!SetEndOfFile(handle))
		{
			uint64_t win32_error_code = GetLastError();
			_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set end of file when extending file: %s. win32 error %llu", copy.data, win32_error_code);
			return 0;
		}

		CloseHandle(handle);
		return 1;
	}
}

uint64_t cf_file_destroy(CC_String path)
{
	CC_String copy = cc_string_copy(path);

	uint64_t success = DeleteFileA(copy.data);

	if (!success)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_DELETION_FAILED, "Failed to delete file: %s.", copy.data);
	}

	cc_string_destroy(copy);

	return success;
}

CF_File *cf_file_open(CC_String path)
{
	CC_String path_copy = cc_string_copy(path);

	// open file handle
	HANDLE handle_file = CreateFileA(path_copy.data, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle_file == INVALID_HANDLE_VALUE)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_OPENING_FAILED, "Failed to open file: %s.", path_copy.data);
		goto path_copy_destroy;
	}
	
	// find file size
	uint64_t file_size;
	GetFileSizeEx(handle_file, (PLARGE_INTEGER)&file_size);

	if (file_size == 0)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_OPENING_FAILED, "Cannot open file with size of 0. %s", path_copy.data);
		goto file_handle_close;
	}

	// open file mapping
	uint32_t high_order_size = (uint32_t)((file_size & 0xFFFFFFFF00000000) >> 32);
	uint32_t low_order_size = (uint32_t)(file_size & 0xFFFFFFFF);
	HANDLE handle_mapping = CreateFileMappingW(handle_file, NULL, PAGE_READWRITE, high_order_size, low_order_size, NULL);
	if (handle_mapping == NULL)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to create file mapping.");
		goto file_handle_close;
	}

	void *data = MapViewOfFile(handle_mapping, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)file_size);
	if (data == NULL)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to open view of file.");
		uint64_t error = GetLastError();
		printf("Error_code = %llu\n", error);
		goto file_handle_mapping_close;
	}

	// allocating CF_File
	CF_File *file = malloc(sizeof(*file));

	file->path = path_copy;
	file->handle_file = handle_file;
	file->handle_mapping = handle_mapping;
	file->size = file_size;
	file->data = data;

	return file;

	// error handling

file_handle_mapping_close:
	CloseHandle(handle_mapping);
file_handle_close:
	CloseHandle(handle_file);
path_copy_destroy:
	cc_string_destroy(path);

	return NULL;
}

uint64_t cf_file_close(CF_File *file)
{
	if (file->data != NULL)
	{
		FlushViewOfFile(file->data, 0);
		UnmapViewOfFile(file->data);
	}

	if (file->handle_mapping != NULL)
	{
		CloseHandle(file->handle_mapping);
	}

	if (file->handle_file != NULL)
	{
		CloseHandle(file->handle_file);
	}

	cc_string_destroy(file->path);

	free(file);

	return 1;
}

uint64_t cf_file_resize(CF_File *file, uint64_t size)
{
	if (file->data != NULL)
	{
		FlushViewOfFile(file->data, 0);
		UnmapViewOfFile(file->data);
	}

	if (file->handle_mapping != NULL)
	{
		CloseHandle(file->handle_mapping);
	}
	
	LARGE_INTEGER li_size = { .QuadPart = size };
	if (!SetFilePointerEx(file->handle_file, li_size, (LARGE_INTEGER *)&size, FILE_BEGIN))
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set file pointer when extending file: %s", file->path.data);
		return 0;
	}

	if (!SetEndOfFile(file->handle_file))
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set end of file when extending file: %s", file->path.data);
		return 0;
	}

	LARGE_INTEGER li_start = { .QuadPart = 0 };
	if (!SetFilePointerEx(file->handle_file, li_start, NULL, FILE_BEGIN))
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set file pointer when extending file: %s", file->path.data);
		return 0;
	}

	file->size = size;

	// open file mapping
	uint32_t high_order_size = (uint32_t)((file->size & 0xFFFFFFFF00000000) >> 32);
	uint32_t low_order_size = (uint32_t)(file->size & 0xFFFFFFFF);
	file->handle_mapping = CreateFileMappingW(file->handle_file, NULL, PAGE_READWRITE, high_order_size, low_order_size, NULL);
	if (file->handle_mapping == NULL)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to create file mapping.");
		return 0;
	}

	file->data = MapViewOfFile(file->handle_mapping, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)file->size);
	if (file->data == NULL)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to open view of file.");
		uint64_t error = GetLastError();
		printf("Error_code = %llu\n", error);
		return 0;
	}

	return 1;
}

uint64_t cf_file_size_get(CF_File *file)
{
	return file->size;
}

#endif