#if defined _WIN32 || defined WIN32

#include "internal.h"

uint64_t cf_file_create(CM_String path)
{
	CM_String copy = cm_string_copy(NULL, path);

	HANDLE handle = CreateFileA(copy.data, GENERIC_READ, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	cm_string_destroy(copy);
	
	if (handle == INVALID_HANDLE_VALUE)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_CREATION_FAILED, "Failed to create file: %s.", copy.data);
		return 0;
	}
	else
	{
		CloseHandle(handle);
		return 1;
	}
}

uint64_t cf_file_destroy(CM_String path)
{
	CM_String copy = cm_string_copy(NULL, path);

	uint64_t success = DeleteFileA(copy.data);

	if (!success)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_DELETION_FAILED, "Failed to delete file: %s.", copy.data);
	}

	cm_string_destroy(copy);

	return success;
}

CF_File *cf_file_open(CM_String path)
{
	CM_String path_copy = cm_string_copy(NULL, path);

	// open file handle
	HANDLE handle_file = CreateFileA(path_copy.data, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle_file == INVALID_HANDLE_VALUE)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_OPENING_FAILED, "Failed to open file: %s.", path_copy.data);
		cm_string_destroy(path_copy);
		return NULL;
	}
	
	// find file size
	uint64_t file_size;
	GetFileSizeEx(handle_file, (PLARGE_INTEGER)&file_size);

	// allocating CF_File
	CF_File *file = malloc(sizeof(*file));

	file->path = path_copy;
	file->handle_file = handle_file;
	file->size = file_size;
	file->file_views = cm_unordered_set_create(sizeof(CF_FileView *), 2);

	return file;
}

uint64_t cf_file_close(CF_File *file)
{
	for (CF_FileView **view = cm_unordered_set_iterator_begin(file->file_views); view != cm_unordered_set_iterator_end(file->file_views); view = cm_unordered_set_iterator_next(file->file_views, view))
	{
		cf_file_view_close(*view);
	}

	if (file->handle_file != NULL)
	{
		CloseHandle(file->handle_file);
	}

	cm_string_destroy(file->path);

	free(file);

	return 1;
}

uint64_t cf_file_resize(CF_File *file, uint64_t size)
{
	for (CF_FileView **view = cm_unordered_set_iterator_begin(file->file_views); view != cm_unordered_set_iterator_end(file->file_views); view = cm_unordered_set_iterator_next(file->file_views, view))
	{
		CF_FileView *_view = *view;
		FlushViewOfFile(_view->data, 0);
		UnmapViewOfFile(_view->data);
		CloseHandle(_view->handle_mapping);
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

	file->size = size;

	for (CF_FileView **view = cm_unordered_set_iterator_begin(file->file_views); view != cm_unordered_set_iterator_end(file->file_views); view = cm_unordered_set_iterator_next(file->file_views, view))
	{
		CF_FileView *_view = *view;
		_view->handle_mapping = CreateFileMapping(file->handle_file, NULL, PAGE_READWRITE, 0, 0, NULL);
		if (_view->handle_mapping == NULL)
		{
			_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to create file mapping.");
			return 0;
		}

		_view->data = MapViewOfFile(_view->handle_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (_view->data == NULL)
		{
			CloseHandle(_view->handle_mapping);
			_view->handle_mapping = NULL;
			_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to open view of file.");
			return 0;
		}
	}

	return 1;
}

uint64_t cf_file_size_get(CF_File *file)
{
	return file->size;
}

#endif