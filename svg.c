#include <stdio.h>
#include "pes.h"

void output_svg(struct pes *pes)
{
	printf("<?xml version=\"1.0\"?>\n");
	printf("<svg xmlns=\"http://www.w3.org/2000/svg\" "
		"xlink=\"http://www.w3.org/1999/xlink\" "
		"ev=\"http://www.w3.org/2001/xml-events\" "
		"version=\"1.1\" "
		"baseProfile=\"full\" "
		"width=\"%d\" height=\"%d\">",
		pes->max_x - pes->min_x,
		pes->max_y - pes->min_y);

	for (struct pes_block *block = pes->blocks; block; block = block->next) {
		if (!block->nr_stitches)
			continue;

		int i;
		printf("<path stroke=\"#%02x%02x%02x\" fill=\"none\" d=\"M %d %d",
			block->color->r,
			block->color->g,
			block->color->b,
			block->stitch[0].x - pes->min_x,
			block->stitch[0].y - pes->min_y);
		for (i = 1; i < block->nr_stitches; i++)
			printf(" L %d %d",
				block->stitch[i].x - pes->min_x,
				block->stitch[i].y - pes->min_y);
		printf("\"/>");
	}
	printf("</svg>\n");
}


