#include "platform/window.h"

// for VlInstance *vl_instance_get(void)
#include "platform/instance.h"

//  for VlWinInstance
#include "instance.h"

#include "memory.h"
#include "utf.h"

#include <Windows.h>
#include <stdio.h>

static char s_initialized = 0;

static const LPCWSTR s_class_name = L"Velours";
static HCURSOR s_pointer_cursor = NULL;

typedef struct {
	HWND hwnd;
} VlWinWindow;

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

VL_API VlWindow *vl_window_new(const u8* title, int x, int y, int w, int h) {
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
	VL_DA(u16) u16_name = utf8_to_utf16(title);
#define DEFAULT(X) (X ? X : CW_USEDEFAULT)
	window->hwnd = CreateWindowEx(
		0,
		s_class_name,
		(LPCWSTR) u16_name,
		WS_OVERLAPPEDWINDOW,
		DEFAULT(w), DEFAULT(h), DEFAULT(x), DEFAULT(y),
		NULL, NULL,
		win->hInstance, NULL
	);
#undef OPTIONAL

	VL_DA_FREE(u16_name);

	if (!window->hwnd) {
		VL_LOG_ERROR("failed to create new window in vl_window_new");
		VL_FREE(window);
		return NULL;
	}
	
	ShowWindow(window->hwnd, win->nCmdShow);

	return (VlWindow*) window;
}

VL_API void vl_window_message_loop(VlWindow *window) {
	if (!window) return;
	VlWinWindow* win = (VlWinWindow*)window;
	MSG msg = { 0 };
	while (GetMessage(&msg, win->hwnd, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

VL_API VlResult vl_window_free(VlWindow* window) {
	if (!window) return VL_ERROR;
	VlWinWindow *win = (VlWinWindow*) window;
	if (!DestroyWindow(win->hwnd)) return VL_ERROR;
	return VL_SUCCESS;
}

static LRESULT CALLBACK vl_window_proc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	switch (msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;

		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH) COLOR_WINDOW);
		EndPaint(hwnd, &ps);
		return 0;
	}
	}
	return DefWindowProc(hwnd, msg, w, l);
}