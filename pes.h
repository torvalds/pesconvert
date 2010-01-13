#ifndef PES_H
#define PES_H

struct color {
	const char *name;
	unsigned char r,g,b;
};

struct stitch {
	int x, y;
};

struct pes_block {
	struct pes_block *next;
	struct color *color;
	int nr_stitches, max_stitches;
	struct stitch stitch[];
};

struct pes {
	int nr_colors;
	int min_x, max_x, min_y, max_y;
	struct pes_block *blocks, *last, **listp;
};

void output_svg(struct pes *pes);
void output_png(struct pes *pes);
void output_cairo(struct pes *pes);

#endif /* PES_H */
