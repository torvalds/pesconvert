#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "pes.h"

/* Function to report an error message to stderr */
static void report(const char *fmt, va_list params)
{
	vfprintf(stderr, fmt, params);
}

/* Function to print an error message and exit the program */
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
	double density = 1.0;
	int arg_index, output_size = -1;
	const char *output_file = NULL;
	struct region region;
	struct pes pes = {
		.min_x = 65535, .max_x = -65535,
		.min_y = 65535, .max_y = -65535,
		.blocks = NULL,
		.last = NULL,
	};

	/* Loop through command line arguments */
	for (arg_index = 1; arg_index < argc; arg_index++) {
		const char *arg = argv[arg_index];

		/* Check if argument is a flag */
		if (*arg == '-') {
			/* Handle flag */
			switch (arg[1]) {
			case 's':
				/* Set output size */
				output_size = atoi(argv[arg_index + 1]);
				if (output_size == 0) {
					die("Invalid output size\n");
				}
				arg_index++;
				continue;
			case 'd':
				/* Set density */
				density = atof(argv[arg_index + 1]);
				if (density == 0.0) {
					die("Invalid density\n");
				}
				arg_index++;
				continue;
			}
			die("Unknown argument '%s'\n", arg);
		}

		/* Check if this is the input file */
		if (!pes.blocks) {
			/* Open input file */
			FILE *input_file = fopen(arg, "r");
			if (!input_file) {
				die("Unable to open input file %s (%s)\n", arg, strerror(errno));
			}

			/* Read region data from input file */
			if (fread(&region, sizeof(struct region), 1, input_file) != 1) {
				die("Unable to read from input file %s (%s)\n", arg, strerror(errno));
			}
				/* Close input file */
		fclose(input_file);

		/* Parse PES data from region data */
		if (parse_pes(&region, &pes) < 0) {
			die("Unable to parse PES file\n");
		}
		continue;
		}

		/* Check if this is the output file */
		if (!output_file) {
			output_file = arg;
			continue;
		}

			/* Too many arguments */
			die("Too many arguments (%s)\n", arg);
		}

		/* Check if input file was specified */
		if (!pes.blocks) {
			die("Need an input PES file\n");
		}

		/* Check if output file was specified */
		if (!output_file) {
			die("Need a png output file name\n");
		}

		/* Generate output image using Cairo library */
		output_cairo(&pes, output_file, output_size, density);

		return 0;

	}
