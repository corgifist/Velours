#include "window.h"
#include "platform/window.h"

// for VlInstance *vl_instance_get(void)
#include "platform/instance.h"

//  for VlWinInstance
#include "instance.h"

#include "memory.h"
#include "utf.h"

#include <stdio.h>

static char s_initialized = 0;

static const LPCWSTR s_class_name = L"Velours";
static HCURSOR s_pointer_cursor = NULL;

static LRESULT CALLBACK vl_window_proc(HWND, UINT, WPARAM, LPARAM);

static void vl_window_initialize(HINSTANCE instance) {
	s_pointer_cursor = LoadCursor(NULL, IDC_ARROW);

	WNDCLASSW w = { 0 };
	w.lpszClassName = s_class_name;
	w.hInstance = instance;
	w.lpfnWndProc = vl_window_proc;
	w.hCursor = s_pointer_cursor;
	RegisterClass(&w);
}

VL_API VlWindow vl_window_new(const u8* title, int x, int y, int w, int h) {
	VlWinInstance* win = (VlWinInstance*) vl_instance_get();
	if (!win) {
		VL_LOG_ERROR("VlWinInstance is NULL\n");
		return NULL;
	}

	if (!s_initialized) {
		vl_window_initialize(win->hInstance);
		s_initialized = 1;
	}

	VlWinWindow *window = VL_MALLOC(sizeof(VlWinWindow));
	if (!window) return NULL;
	if (window) memset(window, 0, sizeof(*window));
	VL_DA(u16) u16_name = utf8_to_utf16(title);
#define DEFAULT(X) (X ? X : CW_USEDEFAULT)
	window->hwnd = CreateWindowEx(
		0,
		s_class_name,
		(LPCWSTR) u16_name,
		WS_OVERLAPPEDWINDOW,
		DEFAULT(x), DEFAULT(y), DEFAULT(w), DEFAULT(h),
		NULL, NULL,
		win->hInstance, window
	);
#undef DEFAULT

	VL_DA_FREE(u16_name);

	if (!window->hwnd) {
		VL_LOG_ERROR("failed to create new window in vl_window_new");
		VL_FREE(window);
		return NULL;
	}

	return (VlWindow) window;
}

VL_API void vl_window_set_visible(VlWindow window, char visible) {
	VlWinInstance *instance = (VlWinInstance*) vl_instance_get();
	if (!instance) {
		VL_LOG_ERROR("VlWinInstance is null");
		return;
	}
	ShowWindow(((VlWinWindow*) window)->hwnd, visible ? SW_SHOW : SW_HIDE);
}

VL_API void vl_window_set_paint_function(VlWindow window, VlWindowPaintFunction fn) {
	if (!window) return;
	((VlWinWindow*) window)->paint_function = fn;
}

VL_API void vl_window_set_resize_function(VlWindow window, VlWindowResizeFunction fn) {
	if (!window) return;
	((VlWinWindow*) window)->resize_function = fn;
}

VL_API void vl_window_set_move_function(VlWindow window, VlWindowMoveFunction fn) {
	if (!window) return;
	((VlWinWindow*) window)->move_function = fn;
}

VL_API void vl_window_invalidate_region(VlWindow window, int x1, int y1, int x2, int y2) {
	if (!window) return;
	VlWinWindow* win = (VlWinWindow*)window;
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		x1 = 0;
		y1 = 0;
		x2 = win->base.cw;
		y2 = win->base.ch;
	}
	RECT r;
	r.left = x1;
	r.top = y1;
	r.right = x2;
	r.bottom = y2;
	InvalidateRect(win->hwnd, &r, FALSE);
}

VL_API void vl_window_message_loop(VlWindow window) {
	if (!window) return;
	VlWinWindow* win = (VlWinWindow*) window;
	MSG msg = { 0 };
	while (GetMessage(&msg, win->hwnd, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

VL_API VlResult vl_window_free(VlWindow window) {
	if (!window) return VL_ERROR;
	ShowWindow(((VlWinWindow*) window)->hwnd, SW_HIDE);
	DestroyWindow(((VlWinWindow*) window)->hwnd);
	VL_FREE(window);
	return VL_SUCCESS;
}

static LRESULT CALLBACK vl_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	if (msg == WM_CREATE) {
		LPCREATESTRUCT cr = (LPCREATESTRUCT) lp;
		VlWindow win = (VlWindow) cr->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) win);
		RECT rect;
		GetWindowRect(hwnd, &rect);
		win->x = rect.left;
		win->y = rect.top;
		win->w = rect.right - rect.left;
		win->h = rect.bottom - rect.top;

		GetClientRect(hwnd, &rect);
		win->cw = rect.right - rect.left;
		win->ch = rect.bottom - rect.top;

		POINT point = { 0, 0 };
		ClientToScreen(hwnd, &point);
		win->cx = point.x;
		win->cy = point.y;

		return 1;
	}

	VlWinWindow *win = (VlWinWindow*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE: {
		if (win) {
			RECT rect;
			GetWindowRect(hwnd, &rect);
			win->base.w = rect.right - rect.left;
			win->base.h = rect.bottom - rect.top;

			GetClientRect(hwnd, &rect);
			win->base.cw = rect.right - rect.left;
			win->base.ch = rect.bottom - rect.top;
		}
		if (win && win->resize_function) {
			win->resize_function((VlWindow) win);
		}
		return 0;
	}
	case WM_MOVE: {
		if (win) {
			RECT rect;
			GetWindowRect(hwnd, &rect);
			win->base.x = rect.left;
			win->base.y = rect.top;

			POINT point = { 0, 0 };
			ClientToScreen(hwnd, &point);
			win->base.cx = point.x;
			win->base.cy = point.y;
		}
		if (win && win->move_function) {
			win->move_function((VlWindow) win);
		}
		return 0;
	}
	case WM_PAINT: {
		if (win->paint_function) {
			win->paint_function((VlWindow) win);
		}
		return 0;
	}
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}