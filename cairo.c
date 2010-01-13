#include <cairo/cairo.h>

#include "pes.h"

#define X(stitch) (((stitch)->x - pes->min_x) * scale)
#define Y(stitch) (((stitch)->y - pes->min_y) * scale)

void output_cairo(struct pes *pes, const char *filename, int size)
{
	int width  = pes->max_x - pes->min_x, outw;
	int height = pes->max_y - pes->min_y, outh;
	double scale = 1.0;
	struct pes_block *block;
	cairo_surface_t *surface;
	cairo_t *cr;

	if (size > 0) {
		int maxd = width > height ? width : height;
		scale = (double) size / maxd;
	}
	outw = width * scale;
	outh = height * scale;

	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, outw+1, outh+1);
	cr = cairo_create (surface);

	block = pes->blocks;
	while (block) {
		struct color *c = block->color;
		struct stitch *stitch = block->stitch;
		int i;

		cairo_set_source_rgb(cr, c->r / 255.0, c->g / 255.0, c->b / 255.0);
		cairo_move_to(cr, X(stitch), Y(stitch));

		for (i = 1; i < block->nr_stitches; i++) {
			++stitch;
			cairo_line_to(cr, X(stitch), Y(stitch));
		}
		cairo_set_line_width(cr, scale);
		cairo_stroke(cr);

		block = block->next;
	}
	cairo_surface_write_to_png(surface, filename);
}
