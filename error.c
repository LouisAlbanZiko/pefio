#include "internal.h"

#define ERROR_STRING_SIZE ((uint64_t)1024)

uint64_t _cf_error_type = 0;
char _cf_error_message[1024];

CF_Error cf_error_message()
{
	CF_Error error =
	{
		.error_type = _cf_error_type,
		.message = {
			.data = _cf_error_message,
			.length = strlen(_cf_error_message)
		}
	};
	return error;
}

void _cf_make_error(uint64_t error_type, const char *format, ...)
{
	_cf_error_type = error_type;

	va_list args;
	va_start(args, format);

	vsprintf_s(_cf_error_message, ERROR_STRING_SIZE, format, args);

	va_end(args);
}