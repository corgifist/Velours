/*
	2d.h - 2d graphics rendering functions
*/

#ifndef VELOURS_PLATFORM_GRAPHICS_H
#define VELOURS_PLATFORM_GRAPHICS_H

#include "velours.h"
#include "window.h"
#include "geometry.h"

#define VL_GRAPHICS_SHOULD_TERMINATE 8

// enum of all available graphics backends
typedef enum {
	VL_GRAPHICS_BACKEND_SOFTWARE = 0,
	VL_GRAPHICS_BACKEND_D2D = 1,
	VL_GRAPHICS_BACKEND_COUNT = 2
} VlGraphicsBackend;

typedef enum {
	VL_GRAPHICS_ANTIALIASING_OFF = 0,
	VL_GRAPHICS_ANTIALIASING_ON = 1,
	VL_GRAPHICS_ANTIALIASING_COUNT = 2
} VlGraphicsAntialiasingMode;

// VlGraphics is a pointer to platform-specific graphic infrastructure
// e.g. VlWinGraphics on windows
struct VlGraphics {
	VlWindow window;
	VlGraphicsBackend type;
	VlGraphicsAntialiasingMode antialias;
};

typedef struct VlGraphics* VlGraphics;

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

typedef VlResult (*VlGraphicsInitializeFunction)(void);
typedef VlGraphics (*VlGraphicsNewFunction)(VlWindow);

typedef VlResult (*VlGraphicsPresentationBeginFunction)(VlGraphics);
typedef VlResult(*VlGraphicsSetAntialiasingModeFunction)(VlGraphics, VlGraphicsAntialiasingMode);
typedef VlResult (*VlGraphicsBeginFunction)(VlGraphics);
typedef VlResult (*VlGraphicsClearFunction)(VlGraphics, VlRGBA);
typedef VlResult(*VlGraphicsLineFunction)(VlGraphics, VlVec2, VlVec2, VlRGBA, int);
typedef VlResult (*VlGraphicsEndFunction)(VlGraphics);
typedef VlResult (*VlGraphicsPresentationEndFunction)(VlGraphics);

typedef VlResult (*VlGraphicsResizeFunction)(VlGraphics, int, int);

typedef VlResult (*VlGraphicsFreeFunction)(VlGraphics);
typedef VlResult (*VlGraphicsTerminateFunction)(void);


// before calling vl_graphics_new, you should call vl_graphics_initialize
// so the graphics backend you would like to use will be ready
// note: you should call vl_graphics_initialize only ONCE
VL_API VlResult vl_graphics_initialize(VlGraphicsBackend backend);

// VlGraphics vl_graphics_new(VlWindow* window)
// creates graphic infrastructure for specific window
VL_API VlGraphics vl_graphics_new(VlWindow window);
VL_API VlResult vl_graphics_free(VlGraphics graphics);

VL_API VlResult vl_graphics_brush_new_solid(VlGraphics graphics, VlGraphicsBrush *brush, VlRGBA rgba);
VL_API void vl_graphics_brush_free(VlGraphicsBrush* brush);

VL_API VlResult vl_graphics_resize(VlGraphics graphics, int w, int h);

VL_API VlResult vl_graphics_presentation_begin(VlGraphics graphics);

VL_API VlResult vl_graphics_set_antialiasing_mode(VlGraphics graphics, VlGraphicsAntialiasingMode mode);

// VlResult vl_graphics_begin(VlGraphics graphics)
// starts drawing on specified VlGraphics
// all rendering functions should be enclosed in vl_graphics_begin / vl_graphics_end function calls
// example:
//     VlGraphics some_graphics;
//     vl_graphics_begin(some_graphics);
//     vl_graphics_clear(some_graphics, ...);
//     vl_graphics_fill_rectangle(some_graphics, ...);
//     vl_graphics_end(some_graphics);
VL_API VlResult vl_graphics_begin(VlGraphics graphics);
VL_API VlResult vl_graphics_clear(VlGraphics graphics, VlRGBA rgba);
VL_API VlResult vl_graphics_line(VlGraphics graphics, VlVec2 p1, VlVec2 p2, VlRGBA brush, int thickness);
VL_API VlResult vl_graphics_draw_rectangle(VlGraphics graphics, VlGraphicsBrush *brush, VlRect rect, float stroke_width);
VL_API VlResult vl_graphics_fill_rectangle(VlGraphics graphics, VlGraphicsBrush* brush, VlRect rect);

// VlResult vl_graphics_end(VlGraphics graphics)
// ends drawing on specified VlGraphics
// usage:
//     see vl_graphics_begin usage
// returns:
//     VL_SUCCESS - successfully ended drawing
//     VL_GRAPHICS_SHOULD_TERMINATE - device has been lost and you should replace your VlGraphics with a new one
//     VL_ERROR - some other unexpected error
VL_API VlResult vl_graphics_end(VlGraphics graphics);

VL_API VlResult vl_graphics_presentation_end(VlGraphics graphics);

// free all internal structures to avoid leaking memory
// you should call this function only ONCE, and only if
// you don't intend to use VlGraphics anymore
VL_API VlResult vl_graphics_terminate(void);

#endif // VELOURS_PLATFORM_GRAPHICS_H