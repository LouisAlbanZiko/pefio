#include "cf_internal.h"

uint64_t CF_File_Load(CF_File *file, uint64_t size, void *data)
{
	if (size == 0)
	{
		size = CF_File_SizeGet(file);
	}
	
	CF_FileView *view = CF_FileView_Open(file, 0, size);
	if (view == NULL)
	{
		return 0;
	}

	uint64_t count = CF_FileView_Read(view, 0, size, data);

	CF_FileView_Close(view);

	return count;
}

uint64_t CF_File_Dump(CF_File *file, uint64_t size, const void *data)
{
	if (size == 0)
	{
		size = CF_File_SizeGet(file);
	}
	else if(size > CF_File_SizeGet(file))
	{
		CF_File_Resize(file, size);
	}

	CF_FileView *view = CF_FileView_Open(file, 0, size);
	if (view == NULL)
	{
		return 0;
	}

	uint64_t count = CF_FileView_Write(view, 0, size, data);

	CF_FileView_Close(view);

	return count;
}