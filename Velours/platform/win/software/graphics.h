#ifndef VELOURS_PLATFORM_WIN_SOFTWARE_GRAPHICS_H
#define VELOURS_PLATFORM_WIN_SOFTWARE_GRAPHICS_H

#include "velours.h"

// for backward-compatibility with VlSoftwarePixelBuffer
#include "platform/universal/software/graphics.h"

// for VlWindow definition
#include "platform/window.h"

typedef struct {
	VlSoftwarePixelBuffer base;
	HBITMAP hbmp;
} VlWinPresentationBuffer;

VlGraphics vl_software_graphics_win_new(VlWindow window);
VlResult vl_software_graphics_win_resize(VlGraphics graphics, int w, int h);
VlResult vl_software_graphics_win_free(VlGraphics graphics);


#endif // VELOURS_PLATFORM_WIN_SOFTWARE_GRAPHICS_H