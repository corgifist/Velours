#include "graphics.h"

#ifdef VL_SOFTWARE_BACKEND_ENABLED
#include "universal/software/graphics.h"
#endif

static VlGraphicsInitializeFunction graphics_initialize = NULL;
static VlGraphicsNewFunction graphics_new = NULL;

static VlGraphicsBeginFunction graphics_begin = NULL;
static VlGraphicsClearFunction graphics_clear = NULL;
static VlGraphicsEndFunction graphics_end = NULL;

static VlGraphicsPresentationBeginFunction graphics_presentation_begin = NULL;
static VlGraphicsPresentationEndFunction graphics_presentation_end = NULL;

static VlGraphicsFreeFunction graphics_free = NULL;
static VlGraphicsTerminateFunction graphics_terminate = NULL;

#define CONCAT(A, B) A ## B
#define GRPTR(BACKEND, NAME) \
	vl_ ## BACKEND ## _graphics_ ## NAME

#define SET_GRAPHICS_POINTERS(B) \
	do { \
		graphics_initialize = GRPTR(B, initialize); \
		graphics_new = GRPTR(B, new); \
		graphics_free = GRPTR(B, free); \
		graphics_terminate = GRPTR(B, terminate); \
	} while (0)

VL_API VlResult vl_graphics_initialize(VlGraphicsBackend backend) {
	// setting rendering & presentation function pointers
#ifdef VL_SOFTWARE_BACKEND_ENABLED
	if (backend == VL_GRAPHICS_BACKEND_SOFTWARE) {
		SET_GRAPHICS_POINTERS(software);
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

VL_API VlResult vl_graphics_free(VlGraphics graphics) {
	if (!graphics_free) return VL_ERROR;
	return graphics_free(graphics);
}

VL_API VlResult vl_graphics_terminate(void) {
	if (!graphics_terminate) return VL_ERROR;

	return graphics_terminate();
}
