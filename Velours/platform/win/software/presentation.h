/*
	software/presentation.h - platform-specific code for presenting
	VlGraphics initialized with VL_GRAPHICS_BACKEND_SOFTWARE to a win32 window
*/

#ifndef VELOURS_PLATFORM_WIN_SOFTWARE_PRESENTATION_H
#define VELOURS_PLATFORM_WIN_SOFTWARE_PRESENTATION_H

#include "velours.h"
#include "platform/graphics.h"

VlResult vl_software_graphics_win_presentation_begin(VlGraphics graphics);
VlResult vl_software_graphics_win_presentation_end(VlGraphics graphics);

#endif // VELOURS_PLATFORM_WIN_SOFTWARE_PRESENTATION_H