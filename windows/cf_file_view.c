#include "internal.h"

CF_FileView *cf_file_view_open(CF_File *file, uint64_t start, uint64_t size)
{
	CF_FileView *view = malloc(sizeof(*view));
	
	view->file = file;
	view->start = start;
	view->size = (size == 0) * (file->size - start) + size;
	view->data = (uint8_t *)file->data + start;

	cc_unordered_set_insert(file->views, &view);

	return view;
}

uint64_t cf_file_view_close(CF_FileView *view)
{
	for (uint64_t i = 0; i < cc_unordered_set_count(view->file->views); i++)
	{
		CF_FileView *_view = *((CF_FileView **)cc_unordered_set_get(view->file->views, i));
		if (view == _view)
		{
			cc_unordered_set_remove(view->file->views, i);
			break;
		}
	}

	free(view);

	return 1;
}

uint64_t cf_file_view_read(CF_FileView *view, uint64_t start, uint64_t size, void *data)
{
	if (view->data != NULL)
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
		_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::view is NULL.");
		return 0;
	}
	
}

uint64_t cf_file_view_write(CF_FileView *view, uint64_t start, uint64_t size, const void *data)
{
	if (view->data != NULL)
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