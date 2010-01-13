/*
 * PES file parsing.
 *
 * All format credit goes to Robert Heel and 
 *
 *	https://bugs.launchpad.net/inkscape/+bug/247463
 *
 * which has a php script to do so. He in turn seems to have
 * gotten it from NJ Crawford's C# PES viewer. I just turned
 * it into C.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "pes.h"

static struct color color_def[256] = {
	{ NULL, 0, 0, 0 },
	{ "Color1",		 14,  31, 124 },
	{ "Color2",		 10,  85, 163 },
	{ "Color3",		 48, 135, 119 },
	{ "Color4",		 75, 107, 175 },
	{ "Color5",		237,  23,  31 },
	{ "Color6",		209,  92,   0 },
	{ "Color7",		145,  54, 151 },
	{ "Color8",		228, 154, 203 },
	{ "Color9",		145,  95, 172 },
	{ "Color10",		157, 214, 125 },
	{ "Color11",		232, 169,   0 },
	{ "Color12",		254, 186,  53 },
	{ "Color13",		255, 255,   0 },
	{ "Color14",		112, 188,  31 },
	{ "Color15",		192, 148,   0 },
	{ "Color16",		168, 168, 168 },
	{ "Color17",		123, 111,   0 },
	{ "Color18",		255, 255, 179 },
	{ "Color19",		 79,  85,  86 },
	{ "Black",		  0,   0,   0 },
	{ "Color21",		 11,  61, 145 },
	{ "Color22",		119,   1, 118 },
	{ "Color23",		 41,  49,  51 },
	{ "Color24",		 42,  19,   1 },
	{ "Color25",		246,  74, 138 },
	{ "Color26",		178, 118,  36 },
	{ "Color27",		252, 187, 196 },
	{ "Color28",		254,  55,  15 },
	{ "White",		240, 240, 240 },
	{ "Color30",		106,  28, 138 },
	{ "Color31",		168, 221, 196 },
	{ "Color32",		 37, 132, 187 },
	{ "Color33",		254, 179,  67 },
	{ "Color34",		255, 240, 141 },
	{ "Color35",		208, 166,  96 },
	{ "Color36",		209,  84,   0 },
	{ "Color37",		102, 186,  73 },
	{ "Color38",		 19,  74,  70 },
	{ "Color39",		135, 135, 135 },
	{ "Color40",		216, 202, 198 },
	{ "Color41",		 67,  86,   7 },
	{ "Color42",		254, 227, 197 },
	{ "Color43",		249, 147, 188 },
	{ "Color44",		  0,  56,  34 },
	{ "Color45",		178, 175, 212 },
	{ "Color46",		104, 106, 176 },
	{ "Color47",		239, 227, 185 },
	{ "Color48",		247,  56, 102 },
	{ "Color49",		181,  76, 100 },
	{ "Color50",		 19,  43,  26 },
	{ "Color51",		199,   1,  85 },
	{ "Color52",		254, 158,  50 },
	{ "Color53",		168, 222, 235 },
	{ "Color54",		  0, 103,  26 },
	{ "Color55",		 78,  41, 144 },
	{ "Color56",		 47, 126,  32 },
	{ "Color57",		253, 217, 222 },
	{ "Color58",		255, 217,  17 },
	{ "Color59",		  9,  91, 166 },
	{ "Color60",		240, 249, 112 },
	{ "Color61",		227, 243,  91 },
	{ "Color62",		255, 200, 100 },
	{ "Color63",		255, 200, 150 },
	{ "Color64",		255, 200, 200 },
};

static struct color *my_colors[256];

struct region {
	const void *ptr;
	unsigned int size;
};

#define CHUNKSIZE (8192)

static int read_file(int fd, struct region *region)
{
	int len = 0, done = 0;
	char *buf = NULL;

	for (;;) {
		int space = len - done, ret;
		if (!space) {
			space = CHUNKSIZE;
			len += space;
			buf = realloc(buf, len);
		}
		ret = read(fd, buf + done, space);
		if (ret > 0) {
			done += ret;
			continue;
		}
		if (!ret)
			break;
		if (errno == EINTR || errno == EAGAIN)
			continue;
		free(buf);
		return -1;
	}

	/* "len+8" guarantees that there is some slop at the end */
	region->ptr = realloc(buf, len+8);
	region->size = len;
	return 0;
}

static int read_path(const char *path, struct region *region)
{
	if (path) {
		int fd = open(path, O_RDONLY);
		if (fd > 0) {
			int ret = read_file(fd, region);
			int saved_errno = errno;
			close(fd);
			errno = saved_errno;
			return ret;
		}
		return fd;
	}
	return read_file(0, region);
}

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

#define get_u8(buf, offset) (*(unsigned char *)((offset)+(const char *)(buf)))
#define get_le32(buf, offset) (*(unsigned int *)((offset)+(const char *)(buf)))

static int parse_pes_colors(struct region *region, unsigned int pec)
{
	const void *buf = region->ptr;
	int nr_colors = get_u8(buf, pec+48) + 1;
	int i;

	for (i = 0; i < nr_colors; i++) {
		struct color *color;
		color = color_def + get_u8(buf, pec+49+i);
		my_colors[i] = color;
	}
	return 0;
}

static struct pes_block *new_block(struct pes *pes)
{
	int size = sizeof(struct pes_block) + 64*sizeof(struct stitch);
	struct pes_block *block = malloc(size);

	if (block) {
		memset(block, 0, size);
		block->max_stitches = 64;

		pes->last = block;
		*pes->listp = block;
		pes->listp = &block->next;
		block->color = my_colors[pes->nr_colors++];
	}
	return block;
}

static int add_stitch(struct pes *pes, int x, int y)
{
	struct pes_block *block = pes->last;
	int nr_stitches = block->nr_stitches;

	if (x < pes->min_x)
		pes->min_x = x;
	if (x > pes->max_x)
		pes->max_x = x;
	if (y < pes->min_y)
		pes->min_y = y;
	if (y > pes->max_y)
		pes->max_y = y;

	if (block->max_stitches == nr_stitches) {
		int new_stitches = (nr_stitches * 3) / 2 + 64;
		int size = sizeof(struct pes_block) + new_stitches*sizeof(struct stitch);
		block = realloc(block, size);
		if (!block)
			return -1;
		block->max_stitches = new_stitches;
	}
	block->stitch[nr_stitches].x = x;
	block->stitch[nr_stitches].y = y;
	block->nr_stitches = nr_stitches+1;
	return 0;
}

static int parse_pes_stitches(struct region *region, unsigned int pec, struct pes *pes)
{
	int oldx, oldy;
	const unsigned char *buf = region->ptr, *p, *end;
	struct pes_block *block;

	p = buf + pec + 532;
	end = buf + region->size;

	oldx = oldy = 0;

	block = new_block(pes);

	while (p < end) {
		int val1 = p[0], val2 = p[1];
		p += 2;
		if (val1 == 255 && !val2)
			return 0;
		if (val1 == 254 && val2 == 176) {
			if (block->nr_stitches) {
				block = new_block(pes);
				if (!block)
					return -1;
			}
			p++;	/* Skip byte */
			continue;
		}

		/* High bit set means 12-bit offset, otherwise 7-bit signed delta */
		if (val1 & 0x80) {
			val1 = ((val1 & 15) << 8) + val2;
			/* Signed 12-bit arithmetic */
			if (val1 & 2048)
				val1 -= 4096;
			val2 = *p++;
		} else {
			if (val1 & 64)
				val1 -= 128;
		}

		if (val2 & 0x80) {
			val2 = ((val2 & 15) << 8) + *p++;
			/* Signed 12-bit arithmetic */
			if (val2 & 2048)
				val2 -= 4096;
		} else {
			if (val2 & 64)
				val2 -= 128;
		}

		val1 += oldx;
		val2 += oldy;

		oldx = val1;
		oldy = val2;

		if (add_stitch(pes, val1, val2))
			return -1;
	}
	return 0;
}

static int parse_pes(struct region *region, struct pes *pes)
{
	const void *buf = region->ptr;
	unsigned int size = region->size;
	unsigned int pec;

	if (size < 48)
		return -1;
	if (memcmp(buf, "#PES", 4))
		return -1;
	pec = get_le32(buf, 8);
	if (pec > region->size)
		return -1;
	if (pec + 532 >= size)
		return -1;
	if (parse_pes_colors(region, pec) < 0)
		return -1;
	return parse_pes_stitches(region, pec, pes);
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
