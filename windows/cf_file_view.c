#include "internal.h"

CF_FileView *cf_file_view_open(CF_File *file, uint64_t start, uint64_t size)
{
	// open file mapping
	uint32_t high_order_size = (uint32_t)((file->size & 0xFFFFFFFF00000000) >> 32);
	uint32_t low_order_size = (uint32_t)(file->size & 0xFFFFFFFF);
	HANDLE handle_mapping = CreateFileMappingW(file->handle_file, NULL, PAGE_READWRITE, high_order_size, low_order_size, NULL);
	if (handle_mapping == NULL)
	{
		_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to create file mapping.");
		return NULL;
	}

	// open view of mapping
	uint32_t high_order_start = (uint32_t)((start & 0xFFFFFFFF00000000) >> 32);
	uint32_t low_order_start = (uint32_t)(start & 0xFFFFFFFF);
	void *data = MapViewOfFile(handle_mapping, FILE_MAP_ALL_ACCESS, high_order_start, low_order_start, (SIZE_T)size);
	if (data == NULL)
	{
		CloseHandle(handle_mapping);
		_cf_make_error(CF_ERROR_TYPE_FILE_VIEW_CREATION_FAILED, "Failed to open view of file.");
		uint64_t error = GetLastError();
		printf("Error_code = %llu\n", error);
		return NULL;
	}


	// allocation CF_FileView
	CF_FileView *view = malloc(sizeof(*view));
	
	view->file = file;
	view->start = start;
	view->size = (size == 0) * file->size + size;
	view->handle_mapping = handle_mapping;
	view->data = data;

	cc_unordered_set_insert(file->file_views, &view);

	return view;
}

uint64_t cf_file_view_close(CF_FileView *view)
{
	for (uint64_t i = 0; i < cc_unordered_set_count(view->file->file_views); i++)
	{
		CF_FileView *_view = *((CF_FileView **)cc_unordered_set_get(view->file->file_views, i));
		if (_view == view)
		{
			cc_unordered_set_remove(view->file->file_views, i);
			break;
		}
	}

	if (view->data != NULL)
	{
		FlushViewOfFile(view->data, 0);
		UnmapViewOfFile(view->data);
	}

	if (view->handle_mapping != NULL)
	{
		CloseHandle(view->handle_mapping);
	}

	free(view);

	return 1;
}

uint64_t cf_file_view_read(CF_FileView *view, uint64_t start, uint64_t size, void *data)
{
	if (view->data != NULL)
	{
		if (view->handle_mapping != NULL)
		{
			if (view->size < start + size || size == 0)
			{
				size = view->size - start;
			}
			memcpy_s(data, size, view->data + start, size);
			return size;
		}
		else
		{
			_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::handle_mapping is NULL.");
			return 0;
		}
	}
	else
	{
		_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::view is NULL.");
		return 0;
	}
	
}

uint64_t cf_file_view_write(CF_FileView *view, uint64_t start, uint64_t size, const void *data)
{
	if (view->data != NULL)
	{
		if (view->handle_mapping != NULL)
		{
			if (view->size < start + size || size == 0)
			{
				size = view->size - start;
			}
			memcpy_s(view->data + start, size, data, size);
			return size;
		}
		else
		{
			_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::handle_mapping is NULL.");
			return 0;
		}
	}
	else
	{
		_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::view is NULL.");
		return 0;
	}
}

uint64_t cf_file_view_flush(CF_FileView *view)
{
	if (view->data != NULL)
	{
		return FlushViewOfFile(view->data, 0);
	}
	else
	{
		_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::data is NULL.");
		return 0;
	}
}

uint64_t cf_file_view_start_get(CF_FileView *view)
{
	return view->start;
}

uint64_t cf_file_view_size_get(CF_FileView *view)
{
	return view->size;
}