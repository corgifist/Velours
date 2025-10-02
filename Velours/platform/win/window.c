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
	window->paint_function = NULL;
	VL_DA(u16) u16_name = utf8_to_utf16(title);
#define DEFAULT(X) (X ? X : CW_USEDEFAULT)
	window->hwnd = CreateWindowEx(
		0,
		s_class_name,
		(LPCWSTR) u16_name,
		WS_OVERLAPPEDWINDOW,
		DEFAULT(w), DEFAULT(h), DEFAULT(x), DEFAULT(y),
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

	return window;
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
	VL_FREE(window);
	return VL_SUCCESS;
}

static LRESULT CALLBACK vl_window_proc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	if (msg == WM_CREATE) {
		LPCREATESTRUCT cr = (LPCREATESTRUCT) l;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) cr->lpCreateParams);
		return 1;
	}

	VlWinWindow *win = (VlWinWindow*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE: {
		UINT width = LOWORD(l);
		UINT height = HIWORD(l);
		if (win->resize_function) {
			win->resize_function(win, (int) width, (int) height);
		}
		return 0;
	}
	case WM_PAINT: {
		if (win->paint_function) {
			win->paint_function(win);
			ValidateRect(hwnd, NULL);
		}
		return 0;
	}
	}
	return DefWindowProc(hwnd, msg, w, l);
}