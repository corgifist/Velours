#ifndef VELOURS_PLATFORM_UNIVERSAL_SOFTWARE_GRAPHICS_H
#define VELOURS_PLATFORM_UNIVERSAL_SOFTWARE_GRAPHICS_H

#include "velours.h"

// for VlGraphics definition
#include "platform/graphics.h"

// for VlWindow definition
#include "platform/window.h"

typedef struct {
	unsigned char *data;
	int w, h;
	char pixel_size;
	char channels, depth;
	size_t row;
} VlSoftwarePixelBuffer;

typedef struct {
	struct VlGraphics base;
	VlSoftwarePixelBuffer *front;
	
	// rendering region of interest
	int x1, y1, x2, y2;

	// width and height of client rendering area
	int w, h;
} VlSoftwareGraphics;

VlResult vl_software_graphics_initialize(void);

VlResult vl_software_graphics_set_antialiasing_mode(VlGraphics graphics, VlGraphicsAntialiasingMode mode);

VlResult vl_software_graphics_begin(VlGraphics graphics);
VlResult vl_software_graphics_clear(VlGraphics graphics, VlRGBA rgba);
VlResult vl_software_graphics_line(VlGraphics graphics, VlVec2 p1, VlVec2 p2, VlRGBA brush, int thickness);
VlResult vl_software_graphics_end(VlGraphics graphics);

VlResult vl_software_graphics_terminate(void);

#endif // VELOURS_PLATFORM_UNIVERSAL_SOFTWARE_GRAPHICS_H bruh