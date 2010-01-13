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
	int i, outputsize = -1;
	const char *output = NULL;
	struct region region;
	struct pes pes = {
		.min_x = 65535, .max_x = -65535,
		.min_y = 65535, .max_y = -65535,
		.blocks = NULL,
		.last = NULL,
		.listp = &pes.blocks,
	};

	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (*arg == '-') {
			switch (arg[1]) {
			case 's':
				outputsize = atoi(argv[i+1]);
				i++;
				continue;
			}
			die("Unknown argument '%s'\n", arg);
		}

		if (!pes.blocks) {
			if (read_path(arg, &region))
				die("Unable to read file %s (%s)\n", arg, strerror(errno));

			if (parse_pes(&region, &pes) < 0)
				die("Unable to parse PES file\n");
			continue;
		}

		if (!output) {
			output = arg;
			continue;
		}

		die("Too many arguments (%s)\n", arg);
	}

	if (!pes.blocks)
		die("Need an input PES file\n");

	if (!output)
		die("Need a png output file name\n");

	output_cairo(&pes, output, outputsize);

	return 0;
}
