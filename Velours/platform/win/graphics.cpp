// on windows we're using Direct2D to handle graphics rendering

#include "platform/graphics.h"
#include "memory.h"

#include "platform/win/window.h"
#include <d2d1.h>

typedef struct {
	ID2D1HwndRenderTarget* hwnd_target;
} VlWinGraphics;

static ID2D1Factory *s_factory = NULL;

VL_API void vl_graphics_initialize(void) {
	VL_HRESULT_CALL(
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &s_factory)
	);
}

VL_API VlGraphics vl_graphics_new(VlWindow window) {
	if (!window) {
		VL_LOG_ERROR("window is null");
		return NULL;
	}
	VlWinWindow *win = (VlWinWindow*) window;

	VlWinGraphics *graphics = (VlWinGraphics*) VL_MALLOC(sizeof(VlWinGraphics));

	RECT r;
	GetClientRect(win->hwnd, &r);

	VL_HRESULT_CALL(
		s_factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(), 
			D2D1::HwndRenderTargetProperties(win->hwnd, D2D1::SizeU((UINT32)r.right - r.left, (UINT32)r.bottom - r.top)), 
			&graphics->hwnd_target
		)
	);

	return (VlGraphics) graphics;
}

VL_API void vl_graphics_free(VlGraphics graphics) {
	if (!graphics) return;
	((VlWinGraphics*) graphics)->hwnd_target->Release();
	VL_FREE(graphics);
}

VL_API void vl_graphics_resize(VlGraphics graphics, int w, int h) {
	if (!graphics) {
		VL_LOG_ERROR("graphics is null");
		return;
	}
	((VlWinGraphics*) graphics)->hwnd_target->Resize(D2D1::SizeU((UINT32) w, (UINT32) h));
}

VL_API VlResult vl_graphics_brush_new_solid(VlGraphics graphics, VlGraphicsBrush* brush, VlRGBA rgba) {
	if (!graphics) {
		VL_LOG_ERROR("graphics is null");
		return NULL;
	}
	VlWinGraphics *win = (VlWinGraphics*) graphics;

	VL_HRESULT_CALL(
		win->hwnd_target->CreateSolidColorBrush(
			D2D1::ColorF(rgba.r, rgba.g, rgba.b, rgba.a),
			(ID2D1SolidColorBrush**) &brush->handle
		)
	);

	brush->owner = graphics;
	brush->type = VL_GRAPHICS_BRUSH_SOLID;

	return VL_SUCCESS;
}

VL_API void vl_graphics_brush_free(VlGraphicsBrush *brush) {
	if (!brush) return;

	((ID2D1Brush*) brush->handle)->Release();
}

VL_API VlResult vl_graphics_begin(VlGraphics graphics) {
	((VlWinGraphics*) graphics)->hwnd_target->BeginDraw();
	return VL_SUCCESS;
}

VL_API VlResult vl_graphics_clear(VlGraphics graphics, VlRGBA rgba) {
	((VlWinGraphics*) graphics)->hwnd_target->Clear(D2D1::ColorF(rgba.r, rgba.g, rgba.b, rgba.a));
	return VL_SUCCESS;
}

VL_API VlResult vl_graphics_draw_rectangle(VlGraphics graphics, VlGraphicsBrush *brush, VlRect rect, float stroke_width) {
	if (!brush) return VL_ERROR;
	((VlWinGraphics*)graphics)->hwnd_target->DrawRectangle(
		D2D1::RectF(rect.x1, rect.y1, rect.x2, rect.y2),
		(ID2D1Brush*)brush->handle,
		stroke_width,
		NULL
	);
	return VL_SUCCESS;
}

VL_API VlResult vl_graphics_end(VlGraphics graphics) {
	VL_HRESULT_CALL(
		((VlWinGraphics*) graphics)->hwnd_target->EndDraw()
	);
	return VL_SUCCESS;
}

VL_API void vl_graphics_terminate(void) {
	s_factory->Release();
}