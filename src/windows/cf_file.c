#if defined _WIN32 || defined WIN32

#include <cf_internal.h>

uint64_t CF_File_Exists(const char *path)
{
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

uint64_t CF_File_Create(const char *path, uint64_t size)
{
	HANDLE handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (handle == INVALID_HANDLE_VALUE)
	{
		uint64_t win32_error_code = GetLastError();
		_cf_make_error(CF_ERROR_TYPE_FILE_CREATION_FAILED, "Failed to create file: %s. win32 error %llu", path, win32_error_code);
		return 0;
	}
	else
	{
		// resize file
		LARGE_INTEGER li_size = { .QuadPart = size };
		if (!SetFilePointerEx(handle, li_size, (LARGE_INTEGER *)&size, FILE_BEGIN))
		{
			uint64_t win32_error_code = GetLastError();
			_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set file pointer when extending file: %s. win32 error %llu", path, win32_error_code);
			return 0;
		}

		if (!SetEndOfFile(handle))
		{
			uint64_t win32_error_code = GetLastError();
			_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set end of file when extending file: %s. win32 error %llu", path, win32_error_code);
			return 0;
		}

		CloseHandle(handle);
		return 1;
	}
}

uint64_t CF_File_Destroy(const char *path)
{
	uint64_t success = DeleteFileA(path);

	if (!success)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_DELETION_FAILED, "Failed to delete file: %s.", path);
	}

	return success;
}

CF_File *CF_File_Open(const char *path)
{
	// open file handle
	HANDLE handle_file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle_file == INVALID_HANDLE_VALUE)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_OPENING_FAILED, "Failed to open file: %s.", path);
		goto path_copy_destroy;
	}
	
	// find file size
	uint64_t file_size;
	GetFileSizeEx(handle_file, (PLARGE_INTEGER)&file_size);

	if (file_size == 0)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_OPENING_FAILED, "Cannot open file with size of 0. %s", path);
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
	CF_File *file = CC_Malloc(sizeof(*file));

	file->path = CC_String_Copy(path);
	file->handle_file = handle_file;
	file->handle_mapping = handle_mapping;
	file->size = file_size;
	file->data = data;
	file->views = CC_UnorderedSet_Create(sizeof(CF_FileView *), 16);

	return file;

	// error handling

file_handle_mapping_close:
	CloseHandle(handle_mapping);
file_handle_close:
	CloseHandle(handle_file);
path_copy_destroy:

	return NULL;
}

uint64_t CF_File_Close(CF_File *file)
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

	CC_String_Destroy(file->path);

	CC_Free(file);

	return 1;
}

uint64_t CF_File_Resize(CF_File *file, uint64_t size)
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
		_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set file pointer when extending file: %s", file->path);
		return 0;
	}

	if (!SetEndOfFile(file->handle_file))
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set end of file when extending file: %s", file->path);
		return 0;
	}

	LARGE_INTEGER li_start = { .QuadPart = 0 };
	if (!SetFilePointerEx(file->handle_file, li_start, NULL, FILE_BEGIN))
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_RESIZE_FAILED, "Failed to set file pointer when extending file: %s", file->path);
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

	for (CF_FileView **view_ptr = CC_UnorderedSet_IteratorBegin(file->views); view_ptr != CC_UnorderedSet_IteratorEnd(file->views); view_ptr = CC_UnorderedSet_IteratorNext(file->views, view_ptr))
	{
		CF_FileView *view = *view_ptr;
		view->data = (uint8_t *)file->data + view->start;
	}

	return 1;
}

uint64_t CF_File_SizeGet(CF_File *file)
{
	return file->size;
}

#endif