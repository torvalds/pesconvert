#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "pes.h"

static void report(const char *fmt, va_list params)
{
	vfprintf(stderr, fmt, params);
}

static void die(const char *fmt, ...)
{
	va_list params;

	va_start(params, fmt);
	report(fmt, params);
	va_end(params);
	exit(1);
}

int main(int argc, char **argv)
{
	struct region region;
	struct pes pes = {
		.min_x = 65535, .max_x = -65535,
		.min_y = 65535, .max_y = -65535,
		.blocks = NULL,
		.last = NULL,
		.listp = &pes.blocks,
	};

	if (read_path(argv[1], &region))
		die("Unable to read file %s (%s)\n", argv[1]?argv[1]:"<stdin>", strerror(errno));

	if (parse_pes(&region, &pes) < 0)
		die("Unable to parse PES file\n");

	output_cairo(&pes);

	return 0;
}
