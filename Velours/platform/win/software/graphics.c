#include "graphics.h"
#include "memory.h"

static VlWinPresentationBuffer* vl_presentation_buffer_new(int w, int h, char channels, char depth) {
	VlWinPresentationBuffer* result = VL_MALLOC(sizeof(VlWinPresentationBuffer));
	result->base.w = w;
	result->base.h = h;
	result->base.channels = channels;
	result->base.depth = depth;
	result->base.pixel_size = channels * depth;
	result->base.row = w * result->base.pixel_size;

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = channels * depth * 8;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(NULL);
	result->hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &result->base.data, NULL, 0);
	ReleaseDC(NULL, hdc);

	return result;
}

static void vl_presentation_buffer_free(VlWinPresentationBuffer* presentation) {
	DeleteObject(presentation->hbmp);
	VL_FREE(presentation);
}

VlGraphics vl_software_graphics_win_new(VlWindow window) {
	if (!window) return NULL;
	VlSoftwareGraphics* software = VL_MALLOC(sizeof(VlSoftwareGraphics));
	if (!software) return NULL;

	software->base.window = window;
	software->base.type = VL_GRAPHICS_BACKEND_SOFTWARE;

	software->front = (VlSoftwarePixelBuffer*) vl_presentation_buffer_new(window->cw, window->ch, 4, 1);
	if (!software->front) {
		VL_FREE(software);
		return NULL;
	}
	software->w = window->cw;
	software->h = window->ch;

	return (VlGraphics) software;
}

VlResult vl_software_graphics_win_resize(VlGraphics graphics, int w, int h) {
	VlSoftwareGraphics *software = (VlSoftwareGraphics*) graphics;
	if (w * h < software->w * software->h) return VL_SUCCESS;
	VlSoftwarePixelBuffer *new_buffer = (VlSoftwarePixelBuffer*) vl_presentation_buffer_new(w, h, 4, 1);
	/*memcpy(new_buffer->data, software->front->data, VL_MIN(
		new_buffer->h * new_buffer->row, 
		software->front->h * software->front->row
	)); */
	vl_presentation_buffer_free((VlWinPresentationBuffer*) software->front);
	software->front = new_buffer;
	software->w = w;
	software->h = h;

	return VL_SUCCESS;
}

VlResult vl_software_graphics_win_free(VlGraphics graphics) {
	VlSoftwareGraphics* software = (VlSoftwareGraphics*)graphics;
	vl_presentation_buffer_free((VlWinPresentationBuffer*) software->front);
	VL_FREE(software);
	return VL_SUCCESS;
}