#ifndef VELOURS_PLATFORM_WIN_WINDOW_H
#define VELOURS_PLATFORM_WIN_WINDOW_H

#include "platform/window.h"
#include <Windows.h>

typedef struct {
	HWND hwnd;
	VlWindowPaintFunction paint_function;
	VlWindowResizeFunction resize_function;
} VlWinWindow;

#endif // VELOURS_PLATFORM_WIN_WINDOW_H