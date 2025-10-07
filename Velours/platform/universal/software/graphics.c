#include "graphics.h"
#include "memory.h"

// VlSoftwarePixelBuffer *vl_software_graphics_pixel_buffer_new(int w, int h, char channels, char depth)
// allocates pixel buffer with specified parameters
// arguments:
//     w - selected width for new pixel buffer
//     h - selected height for new pixel buffer
//     channels - amount of channels per color (4 for RGBA, 3 for RGB and so on)
//     depth - amount of bytes of color channel occupies (for example 1 for RGBA8, 2 for RGBA16 and so on)
static VlSoftwarePixelBuffer *vl_software_graphics_pixel_buffer_new(int w, int h, char channels, char depth) {
	VlSoftwarePixelBuffer* result = VL_MALLOC(sizeof(VlSoftwarePixelBuffer));
	if (!result) return NULL;
	result->w = w;
	result->h = h;
	result->channels = channels;
	result->depth = depth;
	result->pixel_size = channels * depth;
	result->row = w * result->pixel_size;
	result->data = VL_MALLOC(w * h * channels * depth);
	if (!result->data) {
		VL_FREE(result);
		return NULL;
	}
	return result;
}

VlResult vl_software_graphics_initialize(void) {
	// not used by software graphics backend

	return VL_SUCCESS;
}

VlResult vl_software_graphics_begin(VlGraphics graphics) {
	VL_UNUSED(graphics);

	// not used by this graphics backend

	return VL_SUCCESS;
}

VlResult vl_software_graphics_clear(VlGraphics graphics, VlRGBA color) {
	if (!graphics) return VL_ERROR;
	VlSoftwareGraphics *software = (VlSoftwareGraphics*) graphics;
	if (!software->front) return VL_ERROR;
	u8* p = software->front->data;
	char channels = software->front->channels;
	for (int y = software->y1; y < software->y2; y++) {
		for (int x = software->x1; x < software->x2; x++) {
			size_t index = (y * software->front->row) + x * software->front->pixel_size;
			p[index] = (u8) (color.r * 255);
			if (channels >= 2) p[index + 1] = (u8) (color.g * 255);
			if (channels >= 3) p[index + 2] = (u8) (color.b * 255);
			if (channels >= 4) p[index + 3] = (u8) (color.a * 255);
		}
	}
	return VL_SUCCESS;
}

VlResult vl_software_graphics_end(VlGraphics graphics) {
	VL_UNUSED(graphics);

	// not used by this graphics backend

	return VL_SUCCESS;
}

VlResult vl_software_graphics_terminate(void) {
	// not used by software graphics backend

	return VL_SUCCESS;
}