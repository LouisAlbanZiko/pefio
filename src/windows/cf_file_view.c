#if defined _WIN32 || defined WIN32

#include <cf_internal.h>

CF_FileView *CF_FileView_Open(CF_File *file, uint64_t start, uint64_t size)
{
	CF_FileView *view = CC_Malloc(sizeof(*view));
	
	view->file = file;
	view->start = start;
	view->size = (size == 0) * (file->size - start) + size;
	view->data = (uint8_t *)file->data + start;

	CC_UnorderedSet_Insert(file->views, &view);

	return view;
}

uint64_t CF_FileView_Close(CF_FileView *view)
{
	CF_FileView_Flush(view);
	
	for (uint64_t i = 0; i < CC_UnorderedSet_Count(view->file->views); i++)
	{
		CF_FileView *_view = *((CF_FileView **)CC_UnorderedSet_Get(view->file->views, i));
		if (view == _view)
		{
			CC_UnorderedSet_Remove(view->file->views, i);
			break;
		}
	}

	CC_Free(view);

	return 1;
}

uint64_t CF_FileView_Read(CF_FileView *view, uint64_t start, uint64_t size, void *data)
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

uint64_t CF_FileView_Write(CF_FileView *view, uint64_t start, uint64_t size, const void *data)
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

uint64_t CF_FileView_Flush(CF_FileView *view)
{
	if (view->data != NULL)
	{
		return FlushViewOfFile(view->data, view->size);
	}
	else
	{
		_cf_make_error(CF_ERROR_TYPE_NULL_DATA, "CF_FileView::data is NULL.");
		return 0;
	}
}

uint64_t CF_FileView_StartGet(CF_FileView *view)
{
	return view->start;
}

uint64_t CF_FileView_SizeGet(CF_FileView *view)
{
	return view->size;
}

#endif