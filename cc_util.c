#include "internal.h"

uint64_t cf_file_load(CF_File *file, uint64_t size, void *data)
{
	if (size == 0)
	{
		size = cf_file_size_get(file);
	}
	
	CF_FileView *view = cf_file_view_open(file, 0, size);
	if (view == NULL)
	{
		return 0;
	}

	uint64_t count = cf_file_view_read(view, 0, size, data);

	cf_file_view_close(view);

	return count;
}

uint64_t cf_file_dump(CF_File *file, uint64_t size, const void *data)
{
	if (size == 0)
	{
		size = cf_file_size_get(file);
	}

	CF_FileView *view = cf_file_view_open(file, 0, size);
	if (view == NULL)
	{
		return 0;
	}

	uint64_t count = cf_file_view_write(view, 0, size, data);

	cf_file_view_close(view);

	return count;
}