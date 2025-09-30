#include "platform/window.h"

// for VlInstance *vl_instance_get(void)
#include "platform/instance.h"

//  for VlWinInstance
#include "instance.h"

#include <Windows.h>
#include <stdio.h>

static char s_initialized = 0;

static const LPCWSTR s_className = L"Velours";

static LRESULT CALLBACK vl_window_proc(HWND, UINT, WPARAM, LPARAM);

static void vl_window_initialize(HINSTANCE instance) {
	WNDCLASSW w;
	w.lpszClassName = s_className;
	w.hInstance = instance;
	w.lpfnWndProc = vl_window_proc;
	RegisterClass(&w);
}

VL_API VlWindow *vl_window_new(const char *name, int w, int h) {
	VL_UNUSED(name);
	VL_UNUSED(w);
	VL_UNUSED(h);
	if (!s_initialized) {
		VlWinInstance* win = (VlWinInstance*)vl_instance_get();
		if (!win) {
			VL_LOG_ERROR("VlWinInstance is NULL\n");
			return NULL;
		}
		vl_window_initialize(win->instance);
		s_initialized = 1;
	}

	MessageBox(NULL, L"Hello from Velours!", L"Velours", MB_OK);

	return NULL;
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