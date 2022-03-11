#if defined _WIN32 || defined WIN32

#include "internal.h"

uint64_t cf_directory_create(CC_String path)
{
	char *c_path = malloc(path.length + 1);
	for (uint64_t i = 0; i < path.length; i++)
		c_path[i] = path.data[i];
	c_path[path.length] = '\0';

	uint64_t success = CreateDirectoryA(c_path, NULL);

	free(c_path);
	return success;
}

uint64_t cf_directory_destroy(CC_String path)
{
	char *c_path = malloc(path.length + 1);
	for (uint64_t i = 0; i < path.length; i++)
		c_path[i] = path.data[i];
	c_path[path.length] = '\0';

	uint64_t success = RemoveDirectoryA(c_path);

	free(c_path);
	return success;
}

#endif