#include "platform/window.h"

// for VlInstance *vl_instance_get(void)
#include "platform/instance.h"

//  for VlWinInstance
#include "instance.h"

// VL_MALLOC etc.
#include "memory.h"

#include <Windows.h>
#include <stdio.h>

static char s_initialized = 0;

static const LPCWSTR s_class_name = L"Velours";

typedef struct {
	HWND hwnd;
} VlWinWindow;

static LRESULT CALLBACK vl_window_proc(HWND, UINT, WPARAM, LPARAM);

static void vl_window_initialize(HINSTANCE instance) {
	WNDCLASSW w = { 0 };
	w.lpszClassName = s_class_name;
	w.hInstance = instance;
	w.lpfnWndProc = vl_window_proc;
	RegisterClass(&w);
}

VL_API VlWindow *vl_window_new(const char *name, int w, int h) {
	VL_UNUSED(name);
	VL_UNUSED(w);
	VL_UNUSED(h);
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
	window->hwnd = CreateWindowEx(
		0,
		s_class_name,
		(LPCWSTR) name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL,
		win->hInstance, NULL
	);

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