#if defined _WIN32 || defined WIN32

#include <cf_internal.h>

uint64_t CF_Directory_Exists(const char *path)
{
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

uint64_t CF_Directory_Create(const char *path)
{
	uint64_t success = CreateDirectoryA(path, NULL);
	return success;
}

uint64_t CF_Directory_Destroy(const char *path)
{
	uint64_t success = RemoveDirectoryA(path);
	return success;
}

#endif