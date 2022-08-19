#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <c_file/c_file.h>

int main(int argc, char **argv)
{
	const char *file_path = "test.txt";

	if(!CF_File_Exists(file_path))
	{
		CF_File_Create(file_path, 26 * 2);
	}

	CF_File *file = CF_File_Open(file_path);

	{
		CF_FileView *fileView_Lower = CF_FileView_Open(file, 0, 26);

		char data[26];
		for(uint64_t i = 0; i < 26; i++)
		{
			data[i] = 'a' + i;
		}

		CF_FileView_Write(fileView_Lower, 0, 26, data);

		CF_FileView_Close(fileView_Lower);
	}

	{
		CF_FileView *fileView_Upper = CF_FileView_Open(file, 26, 26);

		char data[26];
		for(uint64_t i = 0; i < 26; i++)
		{
			data[i] = 'A' + i;
		}

		CF_FileView_Write(fileView_Upper, 0, 26, data);

		CF_FileView_Close(fileView_Upper);
	}

	CF_File_Close(file);
}
