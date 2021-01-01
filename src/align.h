#ifndef ALIGN_H
#define ALIGN_H

#include <stdio.h>

enum alignment {
	UNKNOWN,
	ALIGNMENT_NONE,
	TOP,
	CENTER,
	BOTTOM,
	LEFT,
	RIGHT,
	HORIZONTAL,
	VERTICAL,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};

#define MAX_NR_WINDOWS (16)

#endif /* ALIGN_H */
