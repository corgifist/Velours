/*
	2d.h - 2d graphics rendering functions
*/

#ifndef VELOURS_PLATFORM_GRAPHICS_H
#define VELOURS_PLATFORM_GRAPHICS_H

#include "velours.h"
#include "window.h"
#include "math.h"

// VlGraphics is a pointer to platform-specific graphic infrastructure
// e.g. VlWinGraphics on windows
typedef void* VlGraphics;

typedef void* VlGraphicsBrushHandle;

typedef enum {
	VL_GRAPHICS_BRUSH_SOLID = 0,
	VL_GRAPHICS_BRUSH_COUNT
} VlGraphicsBrushType;

typedef struct {
	// pointer to the owner of the brush
	VlGraphics owner;

	// brush type, for now only solid brushes are supported
	// but in future we may add linear gradients and so on
	VlGraphicsBrushType type;

	// pointer to platform-specific brush data
	VlGraphicsBrushHandle handle;
} VlGraphicsBrush;

// before calling vl_graphics_new, you should call vl_graphics_initialize
// so all the factories would be ready to be used
// note: you should call vl_graphics_initialize only ONCE
VL_API void vl_graphics_initialize(void);

// VlGraphics* vl_graphics_new(VlWindow* window)
// creates graphic infrastructure for specific window
VL_API VlGraphics vl_graphics_new(VlWindow window);
VL_API void vl_graphics_free(VlGraphics graphics);

VL_API VlResult vl_graphics_brush_new_solid(VlGraphics graphics, VlGraphicsBrush *brush, VlRGBA rgba);
VL_API void vl_graphics_brush_free(VlGraphicsBrush* brush);

VL_API void vl_graphics_resize(VlGraphics graphics, int w, int h);

VL_API VlResult vl_graphics_begin(VlGraphics graphics);
VL_API VlResult vl_graphics_clear(VlGraphics graphics, VlRGBA rgba);
VL_API VlResult vl_graphics_draw_rectangle(VlGraphics graphics, VlGraphicsBrush *brush, VlRect rect, float stroke_width);
VL_API VlResult vl_graphics_end(VlGraphics graphics);

// free all internal structures to avoid leaking memory
// you should call this function only ONCE, and only if
// you don't intend to use VlGraphics anymore
VL_API void vl_graphics_terminate(void);

#endif // VELOURS_PLATFORM_GRAPHICS_H