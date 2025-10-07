#include "presentation.h"

#include "platform/win/software/graphics.h"

// for VlSoftwareGraphics definition
#include "platform/universal/software/graphics.h"

// for VlWinWindow
#include "platform/win/window.h"

static HDC hdc;
static PAINTSTRUCT ps;

VlResult vl_software_graphics_win_presentation_begin(VlGraphics graphics) {
	if (!graphics) return VL_ERROR;
	VlSoftwareGraphics* software = (VlSoftwareGraphics*)graphics;
	VlWinWindow* win = (VlWinWindow*)graphics->window;
	if (!win) return VL_ERROR;

	hdc = BeginPaint(win->hwnd, &ps);

	software->x1 = ps.rcPaint.left;
	software->y1 = ps.rcPaint.top;
	software->x2 = ps.rcPaint.right;
	software->y2 = ps.rcPaint.bottom;

	return VL_SUCCESS;
}

VlResult vl_software_graphics_win_presentation_end(VlGraphics graphics) {
	if (!graphics) return VL_ERROR;
	VlSoftwareGraphics* software = (VlSoftwareGraphics*)graphics;
	VL_UNUSED(software);
	VlWinWindow* win = (VlWinWindow*)graphics->window;
	if (!win) return VL_ERROR;
	VlWinPresentationBuffer* presentation = ((VlWinPresentationBuffer*) software->front);

	HDC hdc_bmp = CreateCompatibleDC(hdc);
	SelectObject(hdc_bmp, presentation->hbmp);

	BitBlt(hdc, software->x1, software->y1, software->x2 - software->x1, software->y2 - software->y1, hdc_bmp, software->x1, software->y1, SRCCOPY);

	DeleteDC(hdc_bmp);
	EndPaint(win->hwnd, &ps);

	return VL_SUCCESS;
}