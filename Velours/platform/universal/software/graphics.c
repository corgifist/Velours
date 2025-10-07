#include "graphics.h"
#include "memory.h"

VlResult vl_software_graphics_initialize(void) {
	// not used by software graphics backend

	return VL_SUCCESS;
}

VlGraphics vl_software_graphics_new(VlWindow window) {
	if (!window) return NULL;
	VlSoftwareGraphics *software = VL_MALLOC(sizeof(VlSoftwareGraphics));
	if (!software) return NULL;

	software->base.window = window;
	software->base.type = VL_GRAPHICS_BACKEND_SOFTWARE;

	return (VlGraphics) software;
}

VlResult vl_software_graphics_free(VlGraphics graphics) {
	VlSoftwareGraphics *software = (VlSoftwareGraphics*) graphics;
	VL_FREE(software);
	return VL_SUCCESS;
}

VlResult vl_software_graphics_terminate(void) {
	// not used by software graphics backend

	return VL_SUCCESS;
}