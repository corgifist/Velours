#ifndef VELOURS_PLATFORM_WINDOW_H
#define VELOURS_PLATFORM_WINDOW_H

#include "velours.h"

// VlWindow is a pointer to platform-specific window structure
// e.g. VlWinWindow on windows

struct VlWindow {
	// w, h - x, y, width and height of a window including title bar and etc.
	int x, y, w, h;

	// cx, cy, cw, ch - x, y, width and height of a window excluding title bar and etc.
	int cx, cy, cw, ch;
};

typedef struct VlWindow* VlWindow;

typedef void (*VlWindowPaintFunction)(VlWindow);
typedef void (*VlWindowResizeFunction)(VlWindow);
typedef void (*VlWindowMoveFunction)(VlWindow);

VL_API VlWindow vl_window_new(const u8 *title, int x, int y, int w, int h);
VL_API void vl_window_set_visible(VlWindow window, char visible);
VL_API void vl_window_set_paint_function(VlWindow window, VlWindowPaintFunction fn);
VL_API void vl_window_set_resize_function(VlWindow window, VlWindowResizeFunction fn);
VL_API void vl_window_set_move_function(VlWindow window, VlWindowMoveFunction fn);

// void vl_window_invalidate_region(VlWindow, int x1, int y1, int x2, int y2)
// manually request a redraw of a specific window region (manually calling paint is not a good idea)
// if x1, y1, x2, y2 are set to 0 then whole window will be invalidated
VL_API void vl_window_invalidate_region(VlWindow, int x1, int y1, int x2, int y2);
VL_API void vl_window_message_loop(VlWindow window);
VL_API VlResult vl_window_free(VlWindow window);

#endif // VELOURS_PLATFORM_WINDOW_H