#include "graphics.h"

#ifdef VL_SOFTWARE_BACKEND_ENABLED
// implementation of software renderer
#include "universal/software/graphics.h"

#ifdef WIN32
// for vl_graphics_software_win_new/free/resize
#include "win/software/graphics.h"
// for vl_graphics_software_win_presentation_xxx
#include "win/software/presentation.h"
#endif // WIN32

#endif

static VlGraphicsInitializeFunction graphics_initialize = NULL;
static VlGraphicsNewFunction graphics_new = NULL;

static VlGraphicsSetAntialiasingModeFunction graphics_set_antialiasing_mode = NULL;
static VlGraphicsPresentationBeginFunction graphics_presentation_begin = NULL;
static VlGraphicsBeginFunction graphics_begin = NULL;
static VlGraphicsClearFunction graphics_clear = NULL;
static VlGraphicsLineFunction graphics_line = NULL;
static VlGraphicsEndFunction graphics_end = NULL;
static VlGraphicsPresentationEndFunction graphics_presentation_end = NULL;

static VlGraphicsResizeFunction graphics_resize = NULL;

static VlGraphicsFreeFunction graphics_free = NULL;
static VlGraphicsTerminateFunction graphics_terminate = NULL;

#define CONCAT(A, B) A ## B
#define GRPTR(BACKEND, NAME) \
	vl_ ## BACKEND ## _graphics_ ## NAME

#define SET_GRAPHICS_POINTERS(B) \
	do { \
		graphics_initialize = GRPTR(B, initialize); \
		graphics_set_antialiasing_mode = GRPTR(B, set_antialiasing_mode); \
		graphics_begin = GRPTR(B, begin); \
		graphics_clear = GRPTR(B, clear); \
		graphics_line = GRPTR(B, line); \
		graphics_end = GRPTR(B, end); \
		graphics_terminate = GRPTR(B, terminate); \
	} while (0)

VL_API VlResult vl_graphics_initialize(VlGraphicsBackend backend) {
	// setting rendering & presentation function pointers
#ifdef VL_SOFTWARE_BACKEND_ENABLED
	if (backend == VL_GRAPHICS_BACKEND_SOFTWARE) {
		SET_GRAPHICS_POINTERS(software);
#ifdef WIN32
		graphics_presentation_begin = vl_software_graphics_win_presentation_begin;
		graphics_presentation_end = vl_software_graphics_win_presentation_end;

		graphics_new = vl_software_graphics_win_new;
		graphics_resize = vl_software_graphics_win_resize;
		graphics_free = vl_software_graphics_win_free;
#endif // WIN32	
	}
#endif // VL_SOFTWARE_BACKEND_ENABLED
	
	if (!graphics_initialize) {
		printf("failed to select suitable graphics backend!\n");
		return VL_ERROR;
	}

	return graphics_initialize();
}

VL_API VlGraphics vl_graphics_new(VlWindow window) {
	if (!graphics_new) return NULL;
	return graphics_new(window);
}

VL_API VlResult vl_graphics_set_antialiasing_mode(VlGraphics graphics, VlGraphicsAntialiasingMode mode) {
	if (!graphics_set_antialiasing_mode) return VL_ERROR;
	return graphics_set_antialiasing_mode(graphics, mode);
}

VL_API VlResult vl_graphics_presentation_begin(VlGraphics graphics) {
	if (!graphics_presentation_begin) return VL_ERROR;
	return graphics_presentation_begin(graphics);
}

VL_API VlResult vl_graphics_begin(VlGraphics graphics) {
	if (!graphics_begin) return VL_ERROR;
	return graphics_begin(graphics);
}

VL_API VlResult vl_graphics_clear(VlGraphics window, VlRGBA rgba) {
	if (!graphics_clear) return VL_ERROR;
	return graphics_clear(window, rgba);
}

VL_API VlResult vl_graphics_line(VlGraphics window, VlVec2 p1, VlVec2 p2, VlRGBA brush, int thickness) {
	if (!graphics_line) return VL_ERROR;
	return graphics_line(window, p1, p2, brush, thickness);
}

VL_API VlResult vl_graphics_end(VlGraphics graphics) {
	if (!graphics_end) return VL_ERROR;
	return graphics_end(graphics);
}

VL_API VlResult vl_graphics_presentation_end(VlGraphics graphics) {
	if (!graphics_presentation_end) return VL_ERROR;
	return graphics_presentation_end(graphics);
}

VL_API VlResult vl_graphics_resize(VlGraphics graphics, int w, int h) {
	if (!graphics_resize) return VL_ERROR;
	return graphics_resize(graphics, w, h);
}

VL_API VlResult vl_graphics_free(VlGraphics graphics) {
	if (!graphics_free) return VL_ERROR;
	return graphics_free(graphics);
}

VL_API VlResult vl_graphics_terminate(void) {
	if (!graphics_terminate) return VL_ERROR;

	return graphics_terminate();
}
