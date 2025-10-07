#ifndef VELOURS_PLATFORM_UNIVERSAL_SOFTWARE_GRAPHICS_H
#define VELOURS_PLATFORM_UNIVERSAL_SOFTWARE_GRAPHICS_H

#include "velours.h"

// for VlGraphics definition
#include "platform/graphics.h"

// for VlWindow definition
#include "platform/window.h"

typedef struct {
	struct VlGraphics base;
} VlSoftwareGraphics;

VlResult vl_software_graphics_initialize(void);
VlGraphics vl_software_graphics_new(VlWindow window);

VlResult vl_software_graphics_free(VlGraphics graphics);
VlResult vl_software_graphics_terminate(void);

#endif // VELOURS_PLATFORM_UNIVERSAL_SOFTWARE_GRAPHICS_H bruh